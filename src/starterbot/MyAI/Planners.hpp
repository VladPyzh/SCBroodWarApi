#pragma once

#include "BB.hpp"
#include "Controller.hpp"

#include <vector>
#include <memory>


struct Behavior {
    virtual void update(const BlackBoard& bb, Controller& controller) = 0;
};

struct Planner {
    Planner() {
        managers = {
            std::make_unique<TrainingBehavior>(),
            std::make_unique<BuildingBehavior>(),
            std::make_unique<HarvestingBehavior>()
        };
    }
    void update(const BlackBoard& bb, Controller& controller) {
        for (auto manager : managers) {
            manager->update(bb, controller);
        }
    }
    std::vector<std::unique_ptr<Behavior>> managers;
};

struct HarvestingBehavior : public Behavior {
    void update(const BlackBoard& bb, Controller& controller) {
        std::vector<Worker> workers = bb.getAvailableWorkers();
        for (const Worker& worker : workers) {
            controller.harvestMinerals(worker);
        }
    }
};

struct BuildingBehavior : public Behavior {
    void update(const BlackBoard& bb, Controller& controller) {
        std::vector<Worker> workers = bb.getAvailableWorkers();
        if (workers.empty()) {
            return;
        }
        // if (shouldBuildBarracks(bb)) {
        //     controller.build(workers[0], BARRACKS);
        // }
        if (shouldBuildSupplyDepot(bb)) {
            controller.build(workers[workers.size() - 1], SUPPLY_DEPOT);
        }
    }

    bool shouldBuildBarracks(const BlackBoard& bb) {
        int minerals = bb.minerals();
        return minarals >= 200;
    }

    bool shouldBuildSupplyDepot(const BlackBoard& bb) {
        int minerals = bb.minerals();
        int unitSlots = bb.unitSlots();

        // Todo: approximate that in X seconds we will outrun of unit slots
        // based on current unit production speed
        return minarals >= 100 && unitSlots < 3;
    }
};

struct TrainingBehavior : public Behavior {
    void update(const BlackBoard& bb, Controller& controller) {
        std::vector<Depot> depots = bb.getDepots();
        for (const Depot& depot : depots) {
            controller.train(depot, WORKER);
        }
        // std::vector<Barrack> barracks = bb.getBarracks();
        // for (const Barrack& barrack : barracks) {
        //     controller.train(barrack, MARINE);
        // }
    }
}