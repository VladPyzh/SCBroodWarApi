#pragma once

#include "BB.hpp"
#include "Controller.hpp"
#include "Units.hpp"
#include "MapTools.h"
#include "bt.hpp"

#include <vector>
#include <memory>


struct Behavior {
    
    struct QuotaRequest {
        int priority;
        int quantity;
        BWAPI::UnitType type;
        int idx = -1;
    };
    virtual QuotaRequest submitQuotaRequest(const BlackBoard& bb) const = 0;
    virtual void update(const BlackBoard& bb, Controller& controller) = 0;
    virtual void assign(std::shared_ptr<BaseUnit> unit, const BlackBoard& bb, Controller& controller) = 0;
    virtual ~Behavior() {}
};


#define DECLARE_STR_TYPE(classname) \
std::string type() { return #classname; } 

template
<typename T>
struct TreeBasedBehavior: public Behavior {
    virtual std::shared_ptr<bt::node> createBT(std::shared_ptr<Unit<T>> unit, const BlackBoard& bb, Controller& controller) = 0;
    virtual std::string type() = 0;
    void assign(std::shared_ptr<BaseUnit> unit, const BlackBoard& bb, Controller& controller) {
        std::shared_ptr<Unit<T>> unit_ptr = std::dynamic_pointer_cast<Unit<T>>(unit);
        unit_ptr->isActive = true;
        std::cerr << unit_ptr->unit->getID() << ' ' << type() << ' ' << '\t';
        trees.push_back(createBT(unit_ptr, bb, controller));
        units.push_back(unit_ptr);
        trees.back()->print();
        std::cerr << '\n';
    }
    void update(const BlackBoard& bb, Controller& controller) {
        std::vector<int> to_delete;
        for (int i = 0; i < trees.size(); i++) {
            auto tree = trees[i];
            auto status = tree->step();
            /*if (type() == std::string("ConstructingBehavior")) {
                trees.back()->print();
                std::cerr << '\n';
                tree->printStack();
                std::cerr << '\n';
            }*/
            if (status == bt::state::failure) {
                std::cerr << units[i]->unit->getID() << ' ' << units[i]->state.inner << '\n';
                tree->print();
                std::cerr << '\n';
                tree->printStack();
                std::cerr << '\n';
                throw std::runtime_error("behavior not allowed to fail");
            }
            if (status != bt::state::running) {
                units[i]->isActive = false;
                trees.erase(trees.begin() + i);
                units.erase(units.begin() + i);
                i--;
                continue;
            }
        }
    }
    std::vector<std::shared_ptr<bt::node>> trees;
    std::vector<std::shared_ptr<Unit<T>>> units;
};

struct GatherMineralsBehavior: public TreeBasedBehavior<WorkerStates> {
    DECLARE_STR_TYPE(GatherMineralsBehavior)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        return QuotaRequest{ 1, 100, BWAPI::UnitTypes::Terran_SCV };
    }
    std::shared_ptr<bt::node> createBT(Worker worker, const BlackBoard& bb, Controller& controller) {
        return bt::sequence({
            bt::one_of({
                bt::sequence({
                    bt::if_true([worker, &controller]() {
                        return controller.harvestMinerals(worker);
                    }),
                    bt::wait_until([worker]() {
                        return worker->state.inner == WorkerStates::W_IS_TO_RETURN_CARGO;
                    })
                }),
                bt::once([&controller, worker]() {
                    controller.moveUnit(worker, BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
                })
            }),
            bt::repeat_until_success([worker, &controller]() {
                return controller.returnCargo(worker);
            }),
            bt::wait_until([worker]() {
                return worker->state.inner == WorkerStates::W_IDLE;
            }),
        });
    }
};

struct ConstructingBehavior : public TreeBasedBehavior<WorkerStates> {
    DECLARE_STR_TYPE(ConstructingBehavior)
    
    int cur_build_index = 0;
    std::vector<BWAPI::UnitType> build_order = {
      BWAPI::UnitTypes::Terran_Supply_Depot,
      BWAPI::UnitTypes::Terran_Barracks,
      BWAPI::UnitTypes::Terran_Supply_Depot,
    };

    std::vector<BWAPI::TilePosition> positions = {
        BWAPI::TilePosition(49, 7),
        BWAPI::TilePosition(46, 11),
        BWAPI::TilePosition(49, 9),
    };

    bool canConstruct(const BlackBoard& bb, BWAPI::UnitType type) const {
        int have_minerals = bb.minerals();
        //int have_gas = bb.gas();
        int cost_mineral = type.mineralPrice();
        int cost_gas = type.gasPrice();

        return have_minerals >= cost_mineral; // && have_gas >= cost_gas;
    }

    bool shouldConstruct(const BlackBoard& bb, BWAPI::UnitType type) const {
        int unitSlots = bb.freeUnitSlots();
        // Todo: approximate that in X seconds we will outrun of unit slots
        // based on current unit production speed
        return canConstruct(bb, type) && unitSlots < 5;
    }

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        if (cur_build_index < build_order.size() && canConstruct(bb, build_order[cur_build_index])) {
            return QuotaRequest{ 5, 1, BWAPI::UnitTypes::Terran_SCV };
        } else if (shouldConstruct(bb, BWAPI::UnitTypes::Terran_Supply_Depot)) {
            return QuotaRequest{ 2, 1, BWAPI::UnitTypes::Terran_SCV };
        } else {
            return QuotaRequest{ 2, 0, BWAPI::UnitTypes::Terran_SCV };
        }
    }

    std::shared_ptr<bt::node> createBT(Worker worker, const BlackBoard& bb, Controller& controller) {
        std::vector<std::shared_ptr<bt::node>> build_order_nodes;
        for (int i = 0; i < build_order.size(); i++) {
            build_order_nodes.push_back(bt::one_of({
                bt::if_true([this, i]() {
                    return cur_build_index > i;
                }),
                bt::sequence({
                    bt::wait_until([&bb = std::as_const(bb), this, i]() {
                        return canConstruct(bb, build_order[i]);
                    }),
                    bt::repeat_until_success([&controller, worker, &bb = std::as_const(bb), this, i]() {
                        return controller.build(worker, build_order[i], positions[i], bb);
                    }),
                    bt::once([this, i]() {
                        BWAPI::Broodwar->printf("Start building: %s", build_order[i].getName().c_str());
                    }),
                    bt::wait_until([worker]() {
                        return worker->state.inner == W_IDLE;
                    })
                })
            }));
        }
        
        std::vector<std::shared_ptr<bt::node>> nodes = {
            bt::one_of({
                bt::if_true([&bb = std::as_const(bb)]() {
                    return bb.m_mapTools.isExplored(BWAPI::Position(47 * 32, 8 * 32));
                }),
                bt::sequence({
                    bt::once([&controller, worker](){
                        controller.moveUnit(worker, BWAPI::Position(47 * 32, 8 * 32));
                    }),
                    bt::wait_until([worker]() {
                        return worker->state.inner == W_IDLE;
                    })
                }),
            }),
            bt::if_true([&bb = std::as_const(bb), this]() {
                return shouldConstruct(bb, BWAPI::UnitTypes::Terran_Supply_Depot);
            }),
            bt::repeat_until_success([worker, &controller, &bb]() {
                BWAPI::TilePosition desiredPos = BWAPI::Broodwar->self()->getStartLocation();
                int maxBuildRange = 64;
                return controller.build(worker, BWAPI::UnitTypes::Terran_Supply_Depot, BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Terran_Supply_Depot, desiredPos, maxBuildRange, false), bb);
            }),
            bt::wait_until([worker]() {
                return worker->state.inner == WorkerStates::W_IDLE;
            })
        };
        nodes.insert(nodes.begin() + 1, build_order_nodes.begin(), build_order_nodes.end());
        return bt::sequence(std::move(nodes));
    }
};

struct ScoutEnemyBaseBehaviour : public TreeBasedBehavior<WorkerStates> {
    DECLARE_STR_TYPE(ScoutEnemyBaseBehaviour)
   
    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        if (!started)
            return QuotaRequest{ 100, 1, BWAPI::UnitTypes::Terran_SCV };
        else
            return QuotaRequest{ 100, 0, BWAPI::UnitTypes::Terran_SCV };
    }
    std::shared_ptr<bt::node> createBT(Worker worker, const BlackBoard& bb, Controller& controller) {
        started = true;
        return bt::once([&controller, worker]() {
            BWAPI::Position start_position = worker->unit->getPosition();
            BWAPI::Position target_position = BWAPI::Position();
            target_position.x = (32 * 96) - start_position.x;
            target_position.y = (32 * 128) - start_position.y;
            controller.moveUnit(worker, target_position);
        });
    }
    bool started = false;
};

struct TrainWorkersBehaviour : public TreeBasedBehavior<DepotStates> {
    DECLARE_STR_TYPE(TrainWorkersBehaviour)
   
    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        return QuotaRequest{ 100, 1 - (int)trees.size(), BWAPI::UnitTypes::Terran_Supply_Depot };
    }
    bool canTrainUnit(const BlackBoard& bb, BWAPI::UnitType type) {
        int minerals = bb.minerals();
        int unitSlots = bb.freeUnitSlots();
        return minerals >= type.mineralPrice() && unitSlots >= type.supplyRequired();
    }
    std::shared_ptr<bt::node> createBT(Depot depot, const BlackBoard& bb, Controller& controller) {
        return bt::repeat(bt::one_of({
            bt::if_true([&bb = std::as_const(bb)]() {
                return bb.m_workers.size() > 6;
            }),
            bt::sequence({
                bt::wait_until([depot]() {
                    return depot->state.inner == D_IDLE;
                }),
                bt::wait_until([&bb = std::as_const(bb), this]() {
                    return canTrainUnit(bb, bb.workerType());
                }),
                bt::repeat_until_success([&controller, depot, &bb = std::as_const(bb)]() {
                    return controller.train(depot, bb.workerType(), bb);
                })
            })
        }));
    }
};

struct TrainMarinesBehaviour : public TreeBasedBehavior<BarrackStates> {
    DECLARE_STR_TYPE(TrainMarinesBehaviour)
    
    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        return QuotaRequest{ 100, 1 - (int)trees.size(), BWAPI::UnitTypes::Terran_Barracks };
    }
    bool canTrainUnit(const BlackBoard& bb, BWAPI::UnitType type) {
        int minerals = bb.minerals();
        int unitSlots = bb.freeUnitSlots();
        return minerals >= type.mineralPrice() && unitSlots >= type.supplyRequired();
    }
    std::shared_ptr<bt::node> createBT(Barrack barrack, const BlackBoard& bb, Controller& controller) {
        return bt::repeat(bt::sequence({
            bt::wait_until([barrack]() {
                return barrack->state.inner == B_IDLE;
            }),
            bt::wait_until([&bb = std::as_const(bb), this]() {
                return canTrainUnit(bb, bb.marineType());
            }),
            bt::repeat_until_success([&controller, barrack, &bb = std::as_const(bb)]() {
                return controller.train(barrack, bb.marineType(), bb);
            })
        }));
    }
};

/*

struct IdleMarineBehavior : public Behavior { // defending logic

    /*
    * Here we more marines to the optimal map position, and say them attack if they can but don't move.
    

    void update(const BlackBoard& bb, Controller& controller) {
        std::cout << "IdleMarineBehavior" << std::endl;
        std::vector<Marine> marines = bb.getMarines();
        for (const marine& barrack : marines) {
            if (marine->unit->isOnPosition()) {
                controller.protect(marine);
            }
            else {
                controller.move_to
            }
        }
    }
};

*/




#undef DECLARE_STR_TYPE
struct Planner {
    Planner(): managers() {
        //managers.emplace_back(std::make_unique<IdleMarineBehavior>());
        // managers.emplace_back(std::make_unique<ScoutingBehavior>());
        // managers.emplace_back(std::make_unique<BuildOrderBehavior>());
        // managers.emplace_back(std::make_unique<FirstScout Behavior>());
        // managers.emplace_back(std::make_unique<TrainingBehavior>());
        // managers.emplace_back(std::make_unique<BuildingBehavior>());
        // managers.emplace_back(std::make_unique<HarvestingBehavior>());
        // managers.emplace_back(std::make_unique<ReturnToBaseBehavior>());

        managers.emplace_back(std::make_unique<GatherMineralsBehavior>());
        managers.emplace_back(std::make_unique<ConstructingBehavior>());
        managers.emplace_back(std::make_unique<ScoutEnemyBaseBehaviour>());
        managers.emplace_back(std::make_unique<TrainMarinesBehaviour>());
        managers.emplace_back(std::make_unique<TrainWorkersBehaviour>());
    }

    template<typename T>
    void assignUnitstoBehavior(std::unique_ptr<Behavior>& b, std::vector<std::shared_ptr<Unit<T>>>& units, Behavior::QuotaRequest quota, const BlackBoard& bb, Controller& controller) {
        int can_provide = std::min(quota.quantity, (int)units.size());
        while (can_provide--) {
            b->assign(static_cast<std::shared_ptr<BaseUnit>>(units.back()), bb, controller);
            units.pop_back();
        }
    }

    void update(const BlackBoard& bb, Controller& controller) {
        std::vector<Behavior::QuotaRequest> requests;
        for (int i = 0; i < managers.size(); i++) {
            auto& manager = managers[i];
            auto q = manager->submitQuotaRequest(bb);
            q.idx = i;
            requests.push_back(q);
        }
        std::sort(requests.rbegin(), requests.rend(), [](Behavior::QuotaRequest a, Behavior::QuotaRequest b) {
            return a.priority < b.priority;
        });
        std::vector<Worker> workers = bb.getUnits(W_IDLE);
        std::vector<Depot> depots = bb.getUnits(D_IDLE);
        std::vector<Barrack> barracks = bb.getUnits(B_IDLE);
        for (int i = 0; i < requests.size(); i++) {
            if (requests[i].type == BWAPI::UnitTypes::Terran_SCV) {
                assignUnitstoBehavior(managers[requests[i].idx], workers, requests[i], bb, controller);
            }
            if (requests[i].type == BWAPI::UnitTypes::Terran_Supply_Depot) {
                assignUnitstoBehavior(managers[requests[i].idx], depots, requests[i], bb, controller);
            }
            if (requests[i].type == BWAPI::UnitTypes::Terran_Barracks) {
                assignUnitstoBehavior(managers[requests[i].idx], barracks, requests[i], bb, controller);
            }
        }
        // distribute vectors of units
        
        for (const auto& manager : managers) {
            manager->update(bb, controller);
        }
       
    }



    std::vector<std::unique_ptr<Behavior>> managers;
};