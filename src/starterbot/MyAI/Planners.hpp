#pragma once

#include "BB.hpp"
#include "Controller.hpp"
#include "Units.hpp"

#include <vector>
#include <memory>


struct Behavior {
    virtual void update(const BlackBoard& bb, Controller& controller) = 0;
    virtual ~Behavior() {}
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
        // if (shouldBuildBarracks(bb)) {
        //     controller.build(workers[0], BARRACKS);
        // }
        BWAPI::Broodwar->printf("A BIBA");

        if (shouldBuildSupplyDepot(bb)) {
            BWAPI::Broodwar->printf("B BIBA");
            controller.build(workersA.back(), BWAPI::UnitTypes::Terran_Supply_Depot);
        }
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
        BWAPI::Broodwar->printf("minerals: %d, unitSlots: %d", minerals, unitSlots);
        return minerals >= BWAPI::UnitTypes::Terran_Supply_Depot.supplyRequired() && unitSlots < 5;
    }
};

struct TrainingBehavior : public Behavior {
    void update(const BlackBoard& bb, Controller& controller) {
        std::vector<Depot> depots = bb.getDepots();
        for (const Depot& depot : depots) {
            controller.train(depot, bb.workerType());
        }
        // std::vector<Barrack> barracks = bb.getBarracks();
        // for (const Barrack& barrack : barracks) {
        //     controller.train(barrack, MARINE);
        // }
    }
};


struct BuildOrderBehavior : public Behavior {
    int cur_build_index = 0;
    std::vector<BWAPI::UnitType> build_order = {
      BWAPI::UnitTypes::Terran_Barracks,  // 1. Barracks
      BWAPI::UnitTypes::Terran_Academy,   // 2. Academy
      BWAPI::UnitTypes::Terran_Barracks,  // 3. Barracks
      BWAPI::UnitTypes::Terran_Factory,   // 4. Factory
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
        int have_gas = bb.gas();
        int cost_mineral = BuildingType.mineralPrice();
        int cost_gas = BuildingType.gasPrice();

        if (have_minerals < cost_mineral || have_gas < cost_gas) {
            BWAPI::Broodwar->printf("\tNot enough minerals or gas");
            return;
        }
        BWAPI::Broodwar->printf("Inserted: %d(+%d)", (int)workers.size(), (int)addWorkers.size());

        if (controller.build(workers.back(), BuildingType)) {
            BWAPI::Broodwar->printf("Start building: %s", BuildingType.getName().c_str());
            cur_build_index++;
        }
    }

};


struct Planner {
    Planner(): managers() {
        managers.emplace_back(std::make_unique<TrainingBehavior>());
        managers.emplace_back(std::make_unique<BuildingBehavior>());
        managers.emplace_back(std::make_unique<BuildOrderBehavior>());
        managers.emplace_back(std::make_unique<HarvestingBehavior>());
    }
    void update(const BlackBoard& bb, Controller& controller) {
        for (const auto& manager : managers) {
            manager->update(bb, controller);
        }
    }
    std::vector<std::unique_ptr<Behavior>> managers;
};