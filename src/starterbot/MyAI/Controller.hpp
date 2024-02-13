



/// Uses bwapi to do some actions
struct Controller {

    void harvestMinerals(Worker worker) {
        BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(worker, BWAPI::Broodwar->getMinerals());
        if (closestMineral) { 
            bool success = worker.unit->gather(closestMineral); 
            if (success) {
                worker.changeState(WorkerStates::MINING);
            }
        }
    }

    void returnCargo(Worker worker) {
        if (worker->returnCargo()) {
            worker.changeState(WorkerStates::RETURNING_CARGO);
        }
    }

    void train(Depot depot, BWAPI::UnitType unitType) {
        if (depot.unit->train(unitType)) {
            depot.changeState(DepotStates::TRAINING);
        }
    }

    void build(Worker worker, BWAPI::UnitType buildingType) {
        // Get a location that we want to build the building next to
        BWAPI::TilePosition desiredPos = BWAPI::Broodwar->self()->getStartLocation();

        // Ask BWAPI for a building location near the desired position for the type
        int maxBuildRange = 64;
        BWAPI::TilePosition buildPos = BWAPI::Broodwar->getBuildLocation(buildingType, desiredPos, maxBuildRange, false);
        if (worker.unit->build(buildingType, buildPos)) {
            worker.changeState(WorkerStates::GOING_TO_BUILD);
        }
    }
};