#pragma once

#include "MapTools.h"
#include "Units.hpp"
#include <vector>
#include <BWAPI.h>
#include <stdexcept>
#include "MapTools.h"
#include "Log.hpp"

constexpr bool CARGO_DEBUG = false;
constexpr bool REFINERY_DEBUG = false;
constexpr bool WORKERS_DEBUG = false;
constexpr bool MARINES_DEBUG = false;
constexpr bool ENEMIES_DEBUG = false;

struct BlackBoard {
    void init() {
        m_mapTools.onStart();
        m_mapTools.saveMapToFile("Destination");
    }

    void fetch() {
        // BWAPI_LOG_IF_ERROR()
        m_mapTools.onFrame();

        for (Worker worker : m_workers) {
            worker->framesSinceUpdate++;
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
                if (worker->unit->isCarryingMinerals() && worker->framesSinceUpdate > 10) {
                    DEBUG_LOG(CARGO_DEBUG, worker->unit->getID() << ' ' << "had cargo in idle" << std::endl)
                    worker->changeState(WorkerStates::W_IS_TO_RETURN_CARGO);
                }
                break;
            }
            case WorkerStates::W_MINING: {
                if (worker->unit->isCarryingMinerals() && worker->framesSinceUpdate > 10) {
                    worker->changeState(WorkerStates::W_IS_TO_RETURN_CARGO);
                    DEBUG_LOG(CARGO_DEBUG, worker->unit->getID() << ' ' << "had cargo in mining" << std::endl)
                }
                break;
            }
            case WorkerStates::W_GASING: {
                //if (worker->unit->isCarryingGas() && worker->framesSinceUpdate > 10) {
                //    worker->changeState(WorkerStates::W_IS_TO_RETURN_CARGO);
                //    DEBUG_LOG(CARGO_DEBUG, worker->unit->getID() << ' ' << "had cargo in gasing" << std::endl)
                //}
                break;
            }
            case WorkerStates::W_GOING_TO_BUILD: {
                if (worker->unit->isConstructing() && worker->framesSinceUpdate > 10) {
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
                    DEBUG_LOG(CARGO_DEBUG, worker->unit->getID() << ' ' << "delivered cargo" << std::endl)
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
            DEBUG_LOG(WORKERS_DEBUG, worker->unit->getID() << ' ' << worker->state.inner << '\n');
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
        for (Refinery ref : m_refineries) {
            if (ref->unit->isBeingConstructed()) {
                ref->changeState(RefineryStates::R_CREATING);
            }
            else if (ref->unit->isCompleted() && ref->unit->getResources() > 0) {
                ref->changeState(RefineryStates::R_IDLE);
            } 
            else if (ref->unit->isCompleted() && ref->unit->getResources() == 0) {
                ref->changeState(RefineryStates::R_EMPTY);
            }
            else {
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
            barrack->highlight();
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
           case MarineStates::M_ATTACKING: {
                if (!marine->unit->isAttacking() && marine->framesSinceUpdate > 10) {
                    marine->changeState(MarineStates::M_IDLE);
                }
                break;
            }
            }
            DEBUG_LOG(MARINES_DEBUG, marine->unit->getID() << ' ' << marine->state.inner << '\n');
        }
        for (Enemy enemy : m_enemies) {
           enemy->highlight();
            if (enemy->unit->isVisible(BWAPI::Broodwar->self())) {
                enemy->changeState(EnemyStates::E_VISIBLE);
            } else if (enemy->framesSinceUpdate > 10) {
                enemy->changeState(EnemyStates::E_UNKNOWN);
            }
            DEBUG_LOG(ENEMIES_DEBUG, enemy->unit->getID() << ' ' << enemy->state.inner << '\n');
        }
        DEBUG_LOG(((ENEMIES_DEBUG && !m_enemies.empty()) || (MARINES_DEBUG && !m_marines.empty()) || (WORKERS_DEBUG && !m_workers.empty())), std::endl);

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

    int gas() const {
        int res = m_gas;
        for (auto type : pending_units) {
            res -= type.gasPrice();
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

    bool haveRefinery() const {
        for (auto ref : m_refineries) {
            if (ref->state.inner == R_IDLE) {
                return true;
            }
        }
        return false;
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

    template<typename T>
    std::vector<std::shared_ptr<Unit<T>>> getUnits(T state) const {
        std::vector<std::shared_ptr<Unit<T>>> res;

        auto units = getUnits<T>();
        for (std::shared_ptr<Unit<T>> unit : units) {
            if (unit->state.inner == state && !unit->isActive) {
                res.push_back(unit);
            }
        }
        return res;
    }

    void removeUnit(int unitId) {
        for (int i = 0; i < m_workers.size(); i++) {
            if (m_workers[i]->unit->getID() == unitId) {
                m_workers[i]->isActive = false;
                m_workers.erase(m_workers.begin() + i);
                return;
            }
        }
        for (int i = 0; i < m_depots.size(); i++) {
            if (m_depots[i]->unit->getID() == unitId) {
                m_depots[i]->isActive = false;
                m_depots.erase(m_depots.begin() + i);
                return;
            }
        }
        for (int i = 0; i < m_supplies.size(); i++) {
            if (m_supplies[i]->unit->getID() == unitId) {
                m_supplies[i]->isActive = false;
                m_supplies.erase(m_supplies.begin() + i);
                return;
            }
        }
        for (int i = 0; i < m_enemies.size(); i++) {
            if (m_enemies[i]->unit->getID() == unitId) {
                m_enemies[i]->isActive = false;
                m_enemies.erase(m_enemies.begin() + i);
                return;
            }
        }
        for (int i = 0; i < m_barracks.size(); i++) {
            if (m_barracks[i]->unit->getID() == unitId) {
                m_barracks[i]->isActive = false;
                m_barracks.erase(m_barracks.begin() + i);
                return;
            }
        }
        for (int i = 0; i < m_marines.size(); i++) {
            if (m_marines[i]->unit->getID() == unitId) {
                m_marines[i]->isActive = false;
                m_marines.erase(m_marines.begin() + i);
                return;
            }
        }
    }

    template<typename T>
    std::vector<std::shared_ptr<Unit<T>>> getUnits() const {
        throw std::runtime_error("not supported");
    }

    template<>
    std::vector<Worker> getUnits<WorkerStates>() const {
        return m_workers;
    }

    template<>
    std::vector<Depot> getUnits<DepotStates>() const {
        return m_depots;
    }

    template<>
    std::vector<Enemy> getUnits<EnemyStates>() const {
        return m_enemies;
    }

    template<>
    std::vector<Barrack> getUnits<BarrackStates>() const {
        return m_barracks;
    }

    template<>
    std::vector<Marine> getUnits<MarineStates>() const {
        return m_marines;
    }

    MapTools m_mapTools;

    std::vector<Worker> m_workers;
    std::vector<Depot> m_depots;
    std::vector<Supply> m_supplies;
    std::vector<Enemy> m_enemies;
    std::vector<Barrack> m_barracks;
    std::vector<Marine> m_marines;
    std::vector<Refinery> m_refineries;

    std::vector<BWAPI::UnitType> pending_units;
    int m_minerals;
    int m_gas;
    int m_unitSlotsAvailable;
    int m_unitSlotsTaken;
};