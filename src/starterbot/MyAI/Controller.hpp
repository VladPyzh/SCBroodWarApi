#pragma once

#include "Units.hpp"
#include "../Tools.h"



/// Uses bwapi to do some actions
struct Controller {

    void harvestMinerals(Worker worker) {
        BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(worker->unit, BWAPI::Broodwar->getMinerals());
        if (closestMineral) { 
            bool success = worker->unit->gather(closestMineral); 
            if (success) {
                worker->changeState(WorkerStates::W_MINING);
            }
        }
    }

    void returnCargo(Worker worker) {
        if (worker->unit->returnCargo()) {
            worker->changeState(WorkerStates::W_RETURNING_CARGO);
        }
    }

    void train(Depot depot, BWAPI::UnitType unitType) {
        if (depot->unit->train(unitType)) {
            depot->changeState(DepotStates::D_TRAINING);
        }
    }

    bool build(Worker worker, BWAPI::UnitType buildingType) {
        // Get a location that we want to build the building next to
        BWAPI::TilePosition desiredPos = BWAPI::Broodwar->self()->getStartLocation();

        // Ask BWAPI for a building location near the desired position for the type
        int maxBuildRange = 64;
        BWAPI::TilePosition buildPos = BWAPI::Broodwar->getBuildLocation(buildingType, desiredPos, maxBuildRange, false);
        if (worker->unit->build(buildingType, buildPos)) {
            worker->changeState(WorkerStates::W_GOING_TO_BUILD);
            return 1;
        }
        else {
            return 0;
        }
    }
};