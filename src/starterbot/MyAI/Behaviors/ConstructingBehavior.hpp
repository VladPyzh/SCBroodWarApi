
#include "BehaviorBase.hpp"

const bool GEYSER_DEBUG = false;

BWAPI::TilePosition findClosestVespeneGeyser(BWAPI::Unit worker) {
    BWAPI::Unit closestGeyser = nullptr;
    double minDistance = std::numeric_limits<double>::max();

    for (auto& unit : BWAPI::Broodwar->getNeutralUnits()) {
        if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
            double distance = worker->getPosition().getDistance(unit->getPosition());
            if (distance < minDistance) {
                closestGeyser = unit;
                minDistance = distance;
            }
        }
    }

    DEBUG_LOG(GEYSER_DEBUG, closestGeyser->getTilePosition().x << " " << closestGeyser->getTilePosition().y << std::endl)


    return closestGeyser->getTilePosition();
}

struct ConstructingBehavior : public TreeBasedBehavior<WorkerStates> {
    DECLARE_STR_TYPE(ConstructingBehavior)

    int cur_build_index = 0;
    std::vector<BWAPI::UnitType> build_order = {
      BWAPI::UnitTypes::Terran_Supply_Depot,
      BWAPI::UnitTypes::Terran_Barracks,
      BWAPI::UnitTypes::Terran_Supply_Depot,
      BWAPI::UnitTypes::Terran_Refinery,
      BWAPI::UnitTypes::Terran_Barracks,
      BWAPI::UnitTypes::Terran_Academy,
    };

    std::vector<BWAPI::TilePosition> positions = {
        BWAPI::TilePosition(49, 7),
        BWAPI::TilePosition(46, 11),
        BWAPI::TilePosition(46, 7),
        BWAPI::TilePosition(0, 0),
        BWAPI::TilePosition(42, 11),
        BWAPI::TilePosition(38, 11),
    };

    bool canConstruct(const BlackBoard& bb, BWAPI::UnitType type) const {
        int have_minerals = bb.minerals();
        int have_gas = bb.gas();
        int cost_mineral = type.mineralPrice();
        int cost_gas = type.gasPrice();

        return have_minerals >= cost_mineral && have_gas >= cost_gas;
    }

    bool shouldConstruct(const BlackBoard& bb, BWAPI::UnitType type) const {
        int unitSlots = bb.freeUnitSlots();
        // Todo: approximate that in X seconds we will outrun of unit slots
        // based on current unit production speed
        return canConstruct(bb, type) && unitSlots < 10;
    }

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        if (cur_build_index < build_order.size() && canConstruct(bb, build_order[cur_build_index])) {
            if (cur_build_index == 0) {
                return QuotaRequest{ 5, std::max(0, 1 - (int)trees.size()), BWAPI::UnitTypes::Terran_SCV };
            }
            else {
                return QuotaRequest{ 5, std::max(0, 2 - (int)trees.size()), BWAPI::UnitTypes::Terran_SCV };
            }
        } else if (cur_build_index >= build_order.size() && shouldConstruct(bb, BWAPI::UnitTypes::Terran_Supply_Depot)) {
            return QuotaRequest{ 2, std::max(0, 5 - (int)trees.size()), BWAPI::UnitTypes::Terran_SCV };
        } else {
            return QuotaRequest{ 2, 0, BWAPI::UnitTypes::Terran_SCV };
        }
    }

    std::shared_ptr<bt::node> createBT(Worker worker, const BlackBoard& bb, Controller& controller) {
        if (cur_build_index < build_order.size()) {
            int tree_for_building_idx = cur_build_index;
            return bt::sequence({
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
                bt::repeat_node_until_success(bt::one_of({
                    bt::if_true([this, tree_for_building_idx]() {
                        // std::cerr << cur_build_index << ' ' << tree_for_building_idx << std::endl;
                        return cur_build_index > tree_for_building_idx;
                    }),
                    bt::sequence({
                        bt::if_true([this, tree_for_building_idx]() {
                            // std::cerr << "chech cur build index " << cur_build_index << ' ' << tree_for_building_idx  << '\n';
                            return cur_build_index == tree_for_building_idx;
                        }),
                        bt::if_true([worker, &bb = std::as_const(bb), this, tree_for_building_idx]() {
                            // auto pos = BWAPI::Broodwar->getBuildLocation(build_order[tree_for_building_idx], positions[tree_for_building_idx], 64, false);

                            if (build_order[tree_for_building_idx] == BWAPI::UnitTypes::Terran_Refinery) { // Corner case for refinery
                                BWAPI::TilePosition position = findClosestVespeneGeyser(worker->unit);
                                bool res = canConstruct(bb, build_order[tree_for_building_idx]) &&
                                    worker->unit->canBuild(build_order[tree_for_building_idx], position);
                                BWAPI_LOG_IF_ERROR()
                                return res;
                            }
                            else {
                                bool res = canConstruct(bb, build_order[tree_for_building_idx]) &&
                                    worker->unit->canBuild(build_order[tree_for_building_idx], positions[tree_for_building_idx]);
                                BWAPI_LOG_IF_ERROR()
                                return res;
                            }
                        }),
                        bt::if_true([&controller, worker, &bb = std::as_const(bb), this, tree_for_building_idx]() {
                            if (build_order[tree_for_building_idx] == BWAPI::UnitTypes::Terran_Refinery) { // Corner case for refinery
                                BWAPI::TilePosition position = findClosestVespeneGeyser(worker->unit);
                                return controller.build(worker, build_order[tree_for_building_idx], position, bb);
                            }
                            else {
                                return controller.build(worker, build_order[tree_for_building_idx], positions[tree_for_building_idx], bb);
                            }
                        }),
                        bt::once([this, tree_for_building_idx]() {
                            cur_build_index++;
                            BWAPI::Broodwar->printf("Start building: %s", build_order[tree_for_building_idx].getName().c_str());
                        }),
                        bt::wait_until([worker]() {
                            // std::cerr << "wait idle " << worker->state.inner << std::endl;
                            // std::cerr << worker->unit->isVisible() << std::endl;
                            return worker->state.inner == W_BUILDING;
                        }),
                        bt::wait_until([worker]() {
                            // std::cerr << "wait idle " << worker->state.inner << std::endl;
                            // std::cerr << worker->unit->isVisible() << std::endl;
                            // if (worker->state.inner != WorkerStates::W_BUILDING) {
                            //     std::cerr << worker->state.inner << std::endl;
                            //      throw std::runtime_error("bad state");
                            // }
                            return worker->state.inner == W_IDLE;
                        })
                    })
                }))
            });
        } else {
            return bt::try_node(    
                bt::sequence({
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
                })
            );
        }
        // std::vector<std::shared_ptr<bt::node>> build_order_nodes;
        // for (int i = 0; i < build_order.size(); i++) {
        //     build_order_nodes.push_back(bt::one_of({
        //         bt::if_true([this, i]() {
        //             // std::cerr << "chech cur build index " << cur_build_index << ' ' << i << '\n';
        //             return cur_build_index > i;
        //         }),
        //         bt::sequence({
        //             bt::wait_until([&bb = std::as_const(bb), this, i]() {
        //                 return canConstruct(bb, build_order[i]);
        //             }),
        //             bt::repeat_until_success([&controller, worker, &bb = std::as_const(bb), this, i]() {
        //                 return controller.build(worker, build_order[i], positions[i], bb);
        //             }),
        //             bt::once([this, i]() {
        //                 cur_build_index++;
        //                 BWAPI::Broodwar->printf("Start building: %s", build_order[i].getName().c_str());
        //             }),
        //             bt::wait_until([worker]() {
        //                 return worker->state.inner == W_IDLE;
        //             })
        //         })
        //     }));
        // }
        
        // std::vector<std::shared_ptr<bt::node>> nodes = {
        //     bt::one_of({
        //         bt::if_true([&bb = std::as_const(bb)]() {
        //             return bb.m_mapTools.isExplored(BWAPI::Position(47 * 32, 8 * 32));
        //         }),
        //         bt::sequence({
        //             bt::once([&controller, worker](){
        //                 controller.moveUnit(worker, BWAPI::Position(47 * 32, 8 * 32));
        //             }),
        //             bt::wait_until([worker]() {
        //                 return worker->state.inner == W_IDLE;
        //             })
        //         }),
        //     }),
        //     bt::if_true([&bb = std::as_const(bb), this]() {
        //         return shouldConstruct(bb, BWAPI::UnitTypes::Terran_Supply_Depot);
        //     }),
        //     bt::repeat_until_success([worker, &controller, &bb]() {
        //         BWAPI::TilePosition desiredPos = BWAPI::Broodwar->self()->getStartLocation();
        //         int maxBuildRange = 64;
        //         return controller.build(worker, BWAPI::UnitTypes::Terran_Supply_Depot, BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Terran_Supply_Depot, desiredPos, maxBuildRange, false), bb);
        //     }),
        //     bt::wait_until([worker]() {
        //         return worker->state.inner == WorkerStates::W_IDLE;
        //     })
        // };
        // nodes.insert(nodes.begin() + 1, build_order_nodes.begin(), build_order_nodes.end());
        // return bt::sequence(std::move(nodes));
    }
};
