#pragma once

#include "BB.hpp"
#include "Controller.hpp"
#include "Units.hpp"
#include "MapTools.h"

#include <vector>
#include <memory>


struct Behavior {
    virtual void update(const BlackBoard& bb, Controller& controller) = 0;
    virtual ~Behavior() {}
};

struct FirstScoutBehavior : public Behavior {
    bool scouted = false;
    void update(const BlackBoard& bb, Controller& controller) {
        std::vector<Worker> workersA = bb.getWorkers(WorkerStates::W_IDLE);
        std::vector<Worker> workersB = bb.getWorkers(WorkerStates::W_RETURNING_CARGO);
        while (workersB.size()) {
            workersA.emplace_back(workersB.back());
            workersB.pop_back();
        }
        if (workersA.empty()) {
            return;
        }

        if (!scouted) {
            std::cout << scouted << std::endl;
            scouted = true;
            controller.moveUnit(workersA.back(), BWAPI::Position(47 * 32, 8 * 32));
        }
    }
};

struct HarvestingBehavior : public Behavior {
    void update(const BlackBoard& bb, Controller& controller) {
        std::vector<Worker> workers = bb.getWorkers(WorkerStates::W_IDLE);
        for (const Worker& worker : workers) {
            controller.harvestMinerals(worker);
        }
        workers = bb.getWorkers(WorkerStates::W_RETURNING_CARGO);
        for (const Worker& worker : workers) {
            controller.returnCargo(worker);
        }
    }
};

struct BuildingBehavior : public Behavior {
    void update(const BlackBoard& bb, Controller& controller) {
        std::vector<Worker> workersA = bb.getWorkers(WorkerStates::W_IDLE);
        std::vector<Worker> workersB = bb.getWorkers(WorkerStates::W_RETURNING_CARGO);
        while (workersB.size()) {
            workersA.emplace_back(workersB.back());
            workersB.pop_back();
        }
        if (workersA.empty()) {
            return;
        }

        if (shouldBuildSupplyDepot(bb) && (bb.m_supplies.size()) < 1) {
            controller.build(workersA.back(), BWAPI::UnitTypes::Terran_Supply_Depot, BWAPI::TilePosition(49, 7));
        }
        else if (shouldBuildSupplyDepot(bb) && (bb.m_supplies.size()) < 2) {
            controller.build(workersA.back(), BWAPI::UnitTypes::Terran_Supply_Depot, BWAPI::TilePosition(49, 8));
        }
        else if (shouldBuildSupplyDepot(bb)) {
            BWAPI::TilePosition desiredPos = BWAPI::Broodwar->self()->getStartLocation();

            int maxBuildRange = 64;
            controller.build(workersA.back(), BWAPI::UnitTypes::Terran_Supply_Depot, BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Terran_Supply_Depot, desiredPos, maxBuildRange, false));
        }
        // if (shouldBuildBarracks(bb)) {
        //     controller.build(workers[0], BARRACKS);
        // }
        // if (shouldBuildSupplyDepot(bb)) {
        //     controller.build(workersA.back(), BWAPI::UnitTypes::Terran_Supply_Depot);
        // }
    }

    bool shouldBuildBarracks(const BlackBoard& bb) {
        int minerals = bb.minerals();
        return minerals >= BWAPI::UnitTypes::Terran_Barracks.supplyRequired();
    }

    bool shouldBuildSupplyDepot(const BlackBoard& bb) {
        int minerals = bb.minerals();
        int unitSlots = bb.freeUnitSlots();

        // Todo: approximate that in X seconds we will outrun of unit slots
        // based on current unit production speed
        return minerals >= BWAPI::UnitTypes::Terran_Supply_Depot.supplyRequired() && unitSlots < 5;
    }
};

struct TrainingBehavior : public Behavior {
    void update(const BlackBoard& bb, Controller& controller) {
        std::vector<Depot> depots = bb.getDepots();
        for (const Depot& depot : depots) {
            if (depot->state.inner == D_TRAINING) {
                continue;
            }
            controller.train(depot, bb.workerType());
        }
        std::vector<Barrack> barracks = bb.getBarracks();
        for (const Barrack& barrack : barracks) {
            if (barrack->state.inner == B_TRAINING) {
                continue;
            }
            controller.train(barrack, bb.marineType());
        }
    }
};

struct BuildOrderBehavior : public Behavior {
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

    void update(const BlackBoard& bb, Controller& controller) {
        if (cur_build_index >= build_order.size())
            return;

        std::vector<Worker> workers = bb.getWorkers(WorkerStates::W_IDLE);
        std::vector<Worker> addWorkers = bb.getWorkers(WorkerStates::W_RETURNING_CARGO);

        workers.insert(workers.end(), addWorkers.begin(), addWorkers.end());


        if (workers.empty()) {
            return;
        }
        const BWAPI::UnitType BuildingType = build_order[cur_build_index];

        int have_minerals = bb.minerals();
        //int have_gas = bb.gas();
        int cost_mineral = BuildingType.mineralPrice();
        int cost_gas = BuildingType.gasPrice();

        if (have_minerals < cost_mineral) {
            BWAPI::Broodwar->printf("\tNot enough minerals or gas");
            return;
        }
        BWAPI::Broodwar->printf("Inserted: %d(+%d)", (int)workers.size(), (int)addWorkers.size());

        if (controller.build(workers.back(), BuildingType, positions[cur_build_index])) {
            BWAPI::Broodwar->printf("Start building: %s", BuildingType.getName().c_str());
            cur_build_index++;
        }
    }

};

struct ScoutingBehavior : public Behavior {
    Worker worker;
    void update(const BlackBoard& bb, Controller& controller) {
        /*
        BWAPI::TilePosition finalScoutingPosition = BWAPI::Broodwar->enemy()->getStartLocation();
        std::cout << "Initial Scouting Position: " << finalScoutingPosition << std::endl;

        if (finalScoutingPosition.y >= 500) {
            finalScoutingPosition.x -= 700;
        }
        else {
            finalScoutingPosition.x += 700;
        }
        std::cout << "Final Scouting Position: " << finalScoutingPosition << std::endl;
        */
        if (!worker) {
            std::vector<Worker> workers = bb.getWorkers(WorkerStates::W_IDLE);
            if (workers.size()) {
                worker = workers.back();
                BWAPI::Position start_position = worker->unit->getPosition();
                BWAPI::Position target_position = BWAPI::Position();
                target_position.x = (32 * 96) - start_position.x;
                target_position.y = (32 * 128) - start_position.y;

                std::cout << "Target Position: " << target_position << std::endl;
                controller.moveUnit(worker, target_position);
            }
        }

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

struct Planner {
    Planner(): managers() {
        //managers.emplace_back(std::make_unique<IdleMarineBehavior>());
        managers.emplace_back(std::make_unique<ScoutingBehavior>());
        managers.emplace_back(std::make_unique<BuildOrderBehavior>());
        managers.emplace_back(std::make_unique<FirstScoutBehavior>());
        managers.emplace_back(std::make_unique<TrainingBehavior>());
        managers.emplace_back(std::make_unique<BuildingBehavior>());
        managers.emplace_back(std::make_unique<HarvestingBehavior>());
    }
    void update(const BlackBoard& bb, Controller& controller) {
        for (const auto& manager : managers) {
            manager->update(bb, controller);
        }
    }
    std::vector<std::unique_ptr<Behavior>> managers;
};