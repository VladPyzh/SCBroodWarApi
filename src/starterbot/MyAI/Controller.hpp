#pragma once

#include "Units.hpp"
#include "../Tools.h"



/// Uses bwapi to do some actions
struct Controller {

    void addPendingUnit(const BlackBoard& bb, BWAPI::UnitType type) {
        BlackBoard& bb_mutable = const_cast<BlackBoard&>(bb);
        bb_mutable.pending_units.emplace_back(type);
    }

    void harvestMinerals(Worker worker) {
        BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(worker->unit, BWAPI::Broodwar->getMinerals());
        if (closestMineral && closestMineral->getResources() > 0) { 
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

    void train(Depot depot, BWAPI::UnitType unitType, const BlackBoard& bb) {
        if (depot->unit->train(unitType)) {
            depot->changeState(DepotStates::D_TRAINING);
            addPendingUnit(bb, unitType);
        }
    }

    void train(Barrack barrack, BWAPI::UnitType unitType, const BlackBoard& bb) {
        if (barrack->unit->train(unitType)) {
            barrack->changeState(BarrackStates::B_TRAINING);
            addPendingUnit(bb, unitType);
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

    bool build(Worker worker, BWAPI::UnitType buildingType, BWAPI::TilePosition buildPos, const BlackBoard& bb) {
        std::cout << "got build order" << std::endl;
        if (worker->unit->build(buildingType, buildPos)) {
            worker->changeState(WorkerStates::W_GOING_TO_BUILD);
            addPendingUnit(bb, buildingType);
            return 1;
        }
        else {
            return 0;
        }
    }

};