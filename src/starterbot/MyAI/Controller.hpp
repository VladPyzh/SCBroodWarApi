#pragma once

#include "Units.hpp"
#include "../Tools.h"

constexpr bool CONTROLLER_DEBUG = false;
constexpr bool CONTROLLER_DEBUG_ATTACK = false;
constexpr bool CONTROLLER_DEBUG_MOVE = false;

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
            DEBUG_LOG(CONTROLLER_DEBUG, "worker unit " << worker->unit->getID() << " returning cargo" << std::endl)
            worker->changeState(WorkerStates::W_RETURNING_CARGO);
            return true;
        }
        return false;
    }

    bool train(Depot depot, BWAPI::UnitType unitType, const BlackBoard& bb) {
        if (depot->unit->train(unitType)) {
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

    void stop(Marine marine) {
        marine->unit->stop();
        DEBUG_LOG(CONTROLLER_DEBUG, "marine stop\n")
        marine->changeState(MarineStates::M_IDLE);
    }

    void moveUnit(Worker worker, BWAPI::Position targetPosition) {
        //moveUnit<WorkerStates>(worker);
        DEBUG_LOG(CONTROLLER_DEBUG, "worker unit " << worker->unit->getID() << "moving \n")
        worker->unit->move(targetPosition);
        worker->changeState(WorkerStates::W_SCOUTING);
    }

    bool moveUnit(Marine marine, BWAPI::Position targetPosition) {
        //moveUnit<WorkerStates>(worker);
        if (marine->unit->move(targetPosition)) {
            marine->changeState(MarineStates::M_MOVING);
            DEBUG_LOG((CONTROLLER_DEBUG || CONTROLLER_DEBUG_MOVE), "marine unit " << marine->unit->getID() << "moving \n")
            return true;
        }
        return false;
    }

    bool moveUnit(Medic medic, BWAPI::Position targetPosition) {
        //moveUnit<WorkerStates>(worker);
        if (medic->unit->move(targetPosition)) {
            medic->changeState(MedicStates::Me_MOVING);
            DEBUG_LOG((CONTROLLER_DEBUG || CONTROLLER_DEBUG_MOVE), "medic unit " << medic->unit->getID() << "moving \n")
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
            return 1;
        }
        else {
            return 0;
        }
    }

    bool attack(Marine marine, Enemy enemy) {
        if (marine->unit->attack(enemy->unit)) {
            enemy->isHighLighted = true;
            DEBUG_LOG((CONTROLLER_DEBUG || CONTROLLER_DEBUG_ATTACK), "marine unit " << marine->unit->getID() << "is attacking " << enemy->unit->getID() << std::endl)
            marine->changeState(MarineStates::M_ATTACKING);
            return 1;
        } else {
            return 0;
        }
    }

};