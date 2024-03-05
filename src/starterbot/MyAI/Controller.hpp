#pragma once

#include "Units.hpp"
#include "../Tools.h"

constexpr bool CONTROLLER_DEBUG = true;

/// Uses bwapi to do some actions
struct Controller {

    void addPendingUnit(const BlackBoard& bb, BWAPI::UnitType type) {
        BlackBoard& bb_mutable = const_cast<BlackBoard&>(bb);
        bb_mutable.pending_units.emplace_back(type);
    }

    bool harvestMinerals(Worker worker) {
        BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(worker->unit, BWAPI::Broodwar->getMinerals());
        if (closestMineral && closestMineral->getResources() > 0) { 
            bool success = worker->unit->gather(closestMineral); 
            DEBUG_LOG(CONTROLLER_DEBUG, "worker unit " << worker->unit->getID() << " gather return code " << success << std::endl)
            BWAPI_LOG_IF_ERROR()
            if (success) {
                worker->changeState(WorkerStates::W_MINING);
            }
            return success;
        }
        return false;
    }

    bool harvestGas(Worker worker) {
        BWAPI::Unit closestRefinery = Tools::GetUnitOfType(BWAPI::UnitTypes::Terran_Refinery);
        if (closestRefinery && closestRefinery->getResources() > 0) {
            bool success = worker->unit->gather(closestRefinery);
            DEBUG_LOG(CONTROLLER_DEBUG, "worker unit " << worker->unit->getID() << " gather return code " << success << std::endl)
                BWAPI_LOG_IF_ERROR()
                if (success) {
                    worker->changeState(WorkerStates::W_GASING);
                }
            return success;
        }
        return false;
    }

    bool returnCargo(Worker worker) {
        if (!worker->unit->isCarryingMinerals()) {
            return true;
        }
        if (worker->unit->returnCargo()) {
            BWAPI_LOG_IF_ERROR()
            DEBUG_LOG(CONTROLLER_DEBUG, "worker unit " << worker->unit->getID() << " returning cargo" << std::endl)
            worker->changeState(WorkerStates::W_RETURNING_CARGO);
            return true;
        }
        BWAPI_LOG_IF_ERROR()
        return false;
    }

    bool train(Depot depot, BWAPI::UnitType unitType, const BlackBoard& bb) {
        if (depot->unit->train(unitType)) {
            DEBUG_LOG(CONTROLLER_DEBUG, "depot training \n")
            depot->changeState(DepotStates::D_TRAINING);
            addPendingUnit(bb, unitType);
            return true;
        }
        return false;
    }

    bool train(Barrack barrack, BWAPI::UnitType unitType, const BlackBoard& bb) {
        if (barrack->unit->train(unitType)) {
            DEBUG_LOG(CONTROLLER_DEBUG, "barrack train\n")
            barrack->changeState(BarrackStates::B_TRAINING);
            addPendingUnit(bb, unitType);
            return true;
        }
        return false;
    }

    void moveUnit(Worker worker, BWAPI::Position targetPosition) {
        //moveUnit<WorkerStates>(worker);
        DEBUG_LOG(CONTROLLER_DEBUG, "worker unit " << worker->unit->getID() << "moving \n")
        worker->unit->move(targetPosition);
        worker->changeState(WorkerStates::W_SCOUTING);
        BWAPI_LOG_IF_ERROR()
    }

    bool moveUnit(Marine marine, BWAPI::Position targetPosition) {
        //moveUnit<WorkerStates>(worker);
        if (marine->unit->move(targetPosition)) {
            marine->changeState(MarineStates::M_MOVING);
            return true;
        }
        return false;
    }

    bool build(Worker worker, BWAPI::UnitType buildingType, BWAPI::TilePosition buildPos, const BlackBoard& bb) {
        DEBUG_LOG(CONTROLLER_DEBUG, "worker unit " << worker->unit->getID() << "wants to build " << buildingType << std::endl)
        if (worker->unit->build(buildingType, buildPos)) {
            DEBUG_LOG(CONTROLLER_DEBUG, "worker unit " << worker->unit->getID() << "is building " << buildingType << std::endl)
            worker->changeState(WorkerStates::W_GOING_TO_BUILD);
            addPendingUnit(bb, buildingType);
            BWAPI_LOG_IF_ERROR()
            return 1;
        }
        else {
            BWAPI_LOG_IF_ERROR()
            return 0;
        }
    }

};