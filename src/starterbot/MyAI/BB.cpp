#include "BB.hpp"


constexpr bool CARGO_DEBUG = false;
constexpr bool REFINERY_DEBUG = false;
constexpr bool WORKERS_DEBUG = false;
constexpr bool MARINES_DEBUG = false;
constexpr bool ACADEMY_DEBUG = false;
constexpr bool ENEMIES_DEBUG = false;

void BlackBoard::init() {
    m_mapTools.onStart();
    m_mapTools.saveMapToFile("Destination");
}

void BlackBoard::fetch() {
    // BWAPI_LOG_IF_ERROR()
    m_mapTools.onFrame();

    for (Worker worker : m_workers) {
        worker->framesSinceUpdate++;
        worker->highlight();
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
        depot->framesSinceUpdate++;
        depot->highlight();
        if (depot->unit->isBeingConstructed()) {
            depot->changeState(DepotStates::D_CREATING);
        } else if (depot->unit->isTraining()) {
            depot->changeState(DepotStates::D_TRAINING);
        } else if (depot->unit->isCompleted()) {
            depot->changeState(DepotStates::D_IDLE);
        } else {
            // throw std::runtime_error("unknown state");
        }
    }
    for (Refinery ref : m_refineries) {
        ref->framesSinceUpdate++;
        ref->highlight();
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
            // throw std::runtime_error("unknown state");
        }
    }
    for (Supply supply : m_supplies) {
        supply->framesSinceUpdate++;
        supply->highlight();
        if (supply->unit->isBeingConstructed()) {
            supply->changeState(SupplyStates::S_CREATING);
        } else if (supply->unit->isCompleted()) {
            supply->changeState(SupplyStates::S_IDLE);
        } else {
            // throw std::runtime_error("unknown state");
        }
    }
    for (Barrack barrack : m_barracks) {
        barrack->framesSinceUpdate++;
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
            // throw std::runtime_error("unknown state");
        }
    }
    for (Academy academy : m_academy) {
        academy->framesSinceUpdate++;
        academy->isHighLighted = true;
        academy->highlight();
        if (academy->unit->isBeingConstructed()) {
            academy->changeState(A_CREATING);
        }
        else if (academy->unit->isUpgrading()) {
            academy->changeState(A_UPGRADING);
        }
        else if (academy->unit->isCompleted()) {
            academy->changeState(A_IDLE);
        }
        else {
            // throw std::runtime_error("unknown state");
        }
        DEBUG_LOG(ACADEMY_DEBUG, academy->unit->getID() << ' ' << academy->state.inner << ' ' << academy->isActive << '\n');
    }
    for (Marine marine : m_marines) {
        marine->framesSinceUpdate++;
        marine->highlight();
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
        DEBUG_LOG(MARINES_DEBUG, marine->unit->getID() << ' ' << marine->state.inner << ' ' << marine->isActive << '\n');
    }
    for (Medic medic : m_medics) {
        switch (medic->state.inner) {
        case MedicStates::Me_UNKNOWN: {
            if (medic->unit->isBeingConstructed()) {
                medic->changeState(MedicStates::Me_CREATING);
            }
            if (medic->unit->isCompleted()) {
                medic->changeState(MedicStates::Me_IDLE);
            }
            break;
        }
        case MedicStates::Me_CREATING: {
            if (medic->unit->isCompleted()) {
                medic->changeState(MedicStates::Me_IDLE);
            }
            break;
        }
        case MedicStates::Me_MOVING: {
            if (!medic->unit->isMoving()) {
                medic->changeState(MedicStates::Me_IDLE);
            }
            break;
        }
        }
    }
    for (Enemy enemy : m_enemies) {
        enemy->highlight();
        enemy->framesSinceUpdate++;
        if (enemy->unit->isVisible(BWAPI::Broodwar->self())) {
            enemy->changeState(EnemyStates::E_VISIBLE);
        } else if (enemy->framesSinceUpdate > 10) {
            enemy->changeState(EnemyStates::E_UNKNOWN);
        }
        DEBUG_LOG(ENEMIES_DEBUG, enemy->unit->getID() << ' ' << enemy->state.inner << '\n');
    }
    DEBUG_LOG(((ENEMIES_DEBUG && !m_enemies.empty()) || (MARINES_DEBUG && !m_marines.empty()) || (WORKERS_DEBUG && !m_workers.empty())), std::endl);

    m_minerals = BWAPI::Broodwar->self()->minerals();
    m_gas = BWAPI::Broodwar->self()->gas();
    m_unitSlotsAvailable = BWAPI::Broodwar->self()->supplyTotal();
    m_unitSlotsTaken = BWAPI::Broodwar->self()->supplyUsed();
}

int BlackBoard::minerals() const {
    int res = m_minerals;
    for (auto type : pending_units) {
        res -= type.mineralPrice();
    }
    return res;
}

int BlackBoard::gas() const {
    int res = m_gas;
    for (auto type : pending_units) {
        res -= type.gasPrice();
    }
    return res;
}

int BlackBoard::freeUnitSlots() const {
    int res = m_unitSlotsAvailable - m_unitSlotsTaken;
    for (auto type : pending_units) {
        res -= type.supplyRequired();
    }
    return res;
}

bool BlackBoard::haveRefinery() const {
    for (auto ref : m_refineries) {
        if (ref->state.inner == R_IDLE) {
            return true;
        }
    }
    return false;
}

int BlackBoard::unitSlotsAvailable() const {
    return m_unitSlotsAvailable;
}

BWAPI::UnitType BlackBoard::workerType() const {
    return BWAPI::Broodwar->self()->getRace().getWorker();
}

BWAPI::UnitType BlackBoard::marineType() const {
    return BWAPI::UnitTypes::Terran_Marine;
}

BWAPI::UnitType BlackBoard::medicType() const {
    return BWAPI::UnitTypes::Terran_Medic;
}

void BlackBoard::removeUnit(int unitId) {
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
            //return;
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
    
    for (int i = 0; i < m_academy.size(); i++) {
        if (m_academy[i]->unit->getID() == unitId) {
            m_academy[i]->isActive = false;
            m_academy.erase(m_academy.begin() + i);
            return;
        }
    }

    for (int i = 0; i < m_medics.size(); i++) {
        if (m_medics[i]->unit->getID() == unitId) {
            m_medics[i]->isActive = false;
            m_medics.erase(m_medics.begin() + i);
            return;
        }
    }
}
