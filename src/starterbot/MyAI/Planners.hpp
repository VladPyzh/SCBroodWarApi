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
            controller.train(depot, bb.workerType());
        }
        // std::vector<Barrack> barracks = bb.getBarracks();
        // for (const Barrack& barrack : barracks) {
        //     controller.train(barrack, MARINE);
        // }
    }
};



struct Planner {
    Planner(): managers() {
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