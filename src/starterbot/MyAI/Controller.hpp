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

    void train(Barrack barrack, BWAPI::UnitType unitType) {
        if (barrack->unit->train(unitType)) {
            barrack->changeState(BarrackStates::B_TRAINING);
        }
    }

    void moveUnit(Worker worker, BWAPI::Position targetPosition) {
        //moveUnit<WorkerStates>(worker);
        worker->unit->move(targetPosition);
        worker->changeState(WorkerStates::W_SCOUTING);
    }

    void moveUnit(Marine marine, BWAPI::Position targetPosition) {
        //moveUnit<WorkerStates>(worker);
        marine->unit->move(targetPosition);
        marine->changeState(MarineStates::M_MOVING);
    }

    bool build(Worker worker, BWAPI::UnitType buildingType, BWAPI::TilePosition buildPos) {
        if (worker->unit->build(buildingType, buildPos)) {
            worker->changeState(WorkerStates::W_GOING_TO_BUILD);
            return 1;
        }
        else {
            return 0;
        }
    }

};