

#include "Unit.hpp"
#include <vector>
#include <stdexcept>


struct BlackBoard {
    void fetch() {
        for (Worker worker : m_workers) {
            if (worker.state.inner == WorkerStates::GOING_TO_BUILD)
                continue;
            if (worker.unit->isCreating()) {
                worker.unit.changeState(WorkerStates::CREATING);
            } else if (worker.unit->isGathering()) {
                worker.unit.changeState(WorkerStates::MINING);
            } else if (worker.unit->isBuilding()) {
                worker.unit.changeState(WorkerStates::BUILDING);
            } else if (worker.unit->isCarryingMinerals()) {
                worker.unit.changeState(WorkerStates::RETURNING_CARGO);
            } else if (worker.unit->isCompleted()) {
                worker.unit.changeState(WorkerStates::IDLE);
            } else {
                throw std::runtime_error("unknown state");
            }
        }
        for (Depot depot : m_depot) {
            if (depot.unit->isCreating()) {
                depot.unit.changeState(DepotStates::CREATING);
            } else if (depot.unit->isTraining()) {
                depot.unit.changeState(DepotStates::TRAINING);
            } else if (depot.unit->isCompleted()) {
                depot.unit.changeState(DepotStates::IDLE);
            } else {
                throw std::runtime_error("unknown state");
            }
        }
        for (Supply supply : m_supplies) {
            if (supply.unit->isCreating()) {
                supply.unit.changeState(SupplyStates::CREATING);
            } else if (supply.unit->isCompleted()) {
                supply.unit.changeState(SupplyStates::IDLE);
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
            res -= type.getMineralPrice();
        }
        return res;
    }

    int unitSlots() const {
        int res = m_unitSlots;
        for (auto type : pending_units) {
            res += type.getSupplyProvided();
        }
        return res;
    }

    void workerType() const {
        return BWAPI::Broodwar->self()->getRace().getWorker();
    }

    std::vector<Worker> m_workers;
    std::vector<Depot> m_depots;
    std::vector<Supply> m_supplies;
    std::vector<BWAPI::UnitType> pending_units;
    int m_minerals;
    int m_unitSlots;
};