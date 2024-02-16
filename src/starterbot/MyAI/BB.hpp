#pragma once

#include "Units.hpp"
#include <vector>
#include <BWAPI.h>
#include <stdexcept>


struct BlackBoard {
    void fetch() {
        for (Worker worker : m_workers) {
            switch (worker->state.inner) {
            case WorkerStates::W_UNKNOWN: {
                if (worker->unit->isBeingConstructed()) {
                    worker->changeState(WorkerStates::W_CREATING);
                }
                if (worker->unit->isCompleted()) {
                    worker->changeState(WorkerStates::W_IDLE);
                }
                break;
            }
            case WorkerStates::W_CREATING: {
                if (worker->unit->isCompleted()) {
                    worker->changeState(WorkerStates::W_IDLE);
                }
                break;
            }
            case WorkerStates::W_IDLE: {
                if (worker->unit->isGatheringMinerals()) {
                    worker->changeState(WorkerStates::W_MINING);
                }
                if (worker->unit->isCarryingMinerals()) {
                    worker->changeState(WorkerStates::W_RETURNING_CARGO);
                }
                break;
            }
            case WorkerStates::W_MINING: {
                if (!worker->unit->isGatheringMinerals() && worker->unit->isCarryingMinerals()) {
                    worker->changeState(WorkerStates::W_RETURNING_CARGO);
                }
                break;
            }
            case WorkerStates::W_GOING_TO_BUILD: {
                if (worker->unit->isConstructing()) {
                    worker->changeState(WorkerStates::W_BUILDING);
                }
                break;
            }
            case WorkerStates::W_BUILDING: {
                if (!worker->unit->isConstructing()) {
                    worker->changeState(WorkerStates::W_IDLE);
                }
                break;
            }
            case WorkerStates::W_RETURNING_CARGO: {
                if (!worker->unit->isCarryingMinerals()) {
                    worker->changeState(WorkerStates::W_IDLE);
                }
                break;
            }
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
    
        for (Worker worker : m_workers) {
            std::cout << worker->unit->getID() << ' ' << worker->state.inner << '\t';
        }
        std::cout << '\n';

        m_minerals = BWAPI::Broodwar->self()->minerals();
        m_unitSlotsAvailable = BWAPI::Broodwar->self()->supplyTotal();
        m_unitSlotsTaken = BWAPI::Broodwar->self()->supplyUsed();
    }

    int minerals() const {
        int res = m_minerals;
        for (auto type : pending_units) {
            res -= type.mineralPrice();
        }
        return res;
    }

    int freeUnitSlots() const {
        int res = m_unitSlotsAvailable - m_unitSlotsTaken;
        for (auto type : pending_units) {
            res += type.supplyProvided();
        }
        for (auto type : pending_units) {
            res -= type.supplyRequired();
        }
        return res;
    }

    int unitSlotsAvailable() const {
        return m_unitSlotsAvailable;
    }

    BWAPI::UnitType workerType() const {
        return BWAPI::Broodwar->self()->getRace().getWorker();
    }

    std::vector<Worker> getWorkers(WorkerStates state) const {
        std::vector<Worker> res;
        for (Worker worker : m_workers) {
            if (worker->state.inner == state) {
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
    int m_unitSlotsAvailable;
    int m_unitSlotsTaken;
};