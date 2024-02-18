#pragma once

#include "MapTools.h"
#include "Units.hpp"
#include <vector>
#include <BWAPI.h>
#include <stdexcept>
#include "MapTools.h"


struct BlackBoard {
    void init() {
        m_mapTools.onStart();
        m_mapTools.saveMapToFile("Destination");
    }

    void fetch() {
        m_mapTools.onFrame();

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
                if (worker->unit->isCarryingMinerals()) {
                    worker->changeState(WorkerStates::W_IS_TO_RETURN_CARGO);
                }
                break;
            }
            case WorkerStates::W_MINING: {
                if (worker->unit->isCarryingMinerals()) {
                    worker->changeState(WorkerStates::W_IS_TO_RETURN_CARGO);
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
            case WorkerStates::W_SCOUTING: {
                if (!worker->unit->isMoving()) {
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
        for (Barrack barrack : m_barracks) {
            if (barrack->unit->isBeingConstructed()) {
                barrack->changeState(B_CREATING);
            }
            else if (barrack->unit->isTraining()) {
                barrack->changeState(B_TRAINING);
            }
            else if (barrack->unit->isCompleted()) {
                barrack->changeState(B_IDLE);
            }
            else {
                throw std::runtime_error("unknown state");
            }
        }
        for (Marine marine : m_marines) {
            switch (marine->state.inner) {
            case MarineStates::M_UNKNOWN: {
                if (marine->unit->isBeingConstructed()) {
                    marine->changeState(MarineStates::M_CREATING);
                }
                if (marine->unit->isCompleted()) {
                    marine->changeState(MarineStates::M_IDLE);
                }
                break;
            }
            case MarineStates::M_CREATING: {
                if (marine->unit->isCompleted()) {
                    marine->changeState(MarineStates::M_IDLE);
                }
                break;
            }
            case MarineStates::M_MOVING: {
                if (!marine->unit->isMoving()) {
                    marine->changeState(MarineStates::M_IDLE);
                }
                break;
            }
            }
        }
    
        
        // for (Worker worker : m_workers) {
        //     std::cout << worker->unit->getID() << ' ' << worker->state.inner << '\t';
        // }
        // std::cout << '\n';
        

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

    BWAPI::UnitType marineType() const {
        return BWAPI::UnitTypes::Terran_Marine;
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

    std::vector<Barrack> getBarracks() const {
        return m_barracks;
    }

    MapTools m_mapTools;

    std::vector<Worker> m_workers;
    std::vector<Depot> m_depots;
    std::vector<Supply> m_supplies;
    //std::vector<Enemy> m_enemy;
    std::vector<Barrack> m_barracks;
    std::vector<Marine> m_marines;

    std::vector<BWAPI::UnitType> pending_units;
    int m_minerals;
    int m_unitSlotsAvailable;
    int m_unitSlotsTaken;
};