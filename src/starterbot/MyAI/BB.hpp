#pragma once

#include "Units.hpp"
#include <vector>
#include <BWAPI.h>
#include <stdexcept>


struct BlackBoard {
    void fetch() {
        for (Worker worker : m_workers) {
            if (worker->state.inner == WorkerStates::W_GOING_TO_BUILD)
                continue;
            if (worker->unit->isBeingConstructed()) {
                worker->changeState(WorkerStates::W_CREATING);
            } else if (worker->unit->isGatheringMinerals()) {
                worker->changeState(WorkerStates::W_MINING);
            } else if (worker->unit->isConstructing()) {
                worker->changeState(WorkerStates::W_BUILDING);
            } else if (worker->unit->isCarryingMinerals()) {
                worker->changeState(WorkerStates::W_RETURNING_CARGO);
            } else if (worker->unit->isCompleted()) {
                worker->changeState(WorkerStates::W_IDLE);
            } else {
                throw std::runtime_error("unknown state");
            }
        }
        for (Depot depot : m_depots) {
            if (depot->unit->isBeingConstructed()) {
                depot->changeState(DepotStates::D_CREATING);
            } else if (depot->unit->isTraining()) {
                depot->changeState(DepotStates::D_TRAINING);
            } else if (depot->unit->isCompleted()) {
                depot->changeState(DepotStates::D_IDLE);
            } else {
                throw std::runtime_error("unknown state");
            }
        }
        for (Supply supply : m_supplies) {
            if (supply->unit->isBeingConstructed()) {
                supply->changeState(SupplyStates::S_CREATING);
            } else if (supply->unit->isCompleted()) {
                supply->changeState(SupplyStates::S_IDLE);
            } else {
                throw std::runtime_error("unknown state");
            }
        }
    
        m_minerals = BWAPI::Broodwar->self()->minerals();
        m_unitSlots = BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed();
    }

    int minerals() const {
        int res = m_minerals;
        for (auto type : pending_units) {
            res -= type.mineralPrice();
        }
        return res;
    }

    int unitSlots() const {
        int res = m_unitSlots;
        for (auto type : pending_units) {
            res += type.supplyProvided();
        }
        return res;
    }

    BWAPI::UnitType workerType() const {
        return BWAPI::Broodwar->self()->getRace().getWorker();
    }

    std::vector<Worker> getAvailableWorkers() const {
        std::vector<Worker> res;
        for (Worker worker : m_workers) {
            if (worker->state.inner == WorkerStates::W_IDLE) {
                res.push_back(worker);
            }
        }
        return res;
    }

    std::vector<Depot> getDepots() const {
        return m_depots;
    }

    std::vector<Worker> m_workers;
    std::vector<Depot> m_depots;
    std::vector<Supply> m_supplies;
    std::vector<BWAPI::UnitType> pending_units;
    int m_minerals;
    int m_unitSlots;
};