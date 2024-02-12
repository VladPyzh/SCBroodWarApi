

#include "Unit.hpp"
#include <vector>


struct BlackBoard {
    void fetch() {
        for (Worker worker : m_workers) {
            if (worker.unit->isCreating()) {
                worker.unit.changeState(CREATING);
            }
            if (worker.unit->isCompleted()) {
                worker.unit.changeState(IDLE);
            }
            if (worker.unit->isGathering()) {
                worker.unit.changeState(MINING);
            }
        }
        for (Depot depot : m_depot) {
            if (depot.unit->isCreating()) {
                depot.unit.changeState(CREATING);
            }
            if (depot.unit->isCompleted()) {
                depot.unit.changeState(IDLE);
            }
            if (depot.unit->isTraining()) {
                depot.unit.changeState(TRAINING);
            }
        }
        for (Supply supply : m_supplies) {
            if (supply.unit->isCreating()) {
                supply.unit.changeState(CREATING);
            }
            if (supply.unit->isCompleted()) {
                supply.unit.changeState(IDLE);
            }
        }
    
        m_minerals = BWAPI::Broodwar->self()->minerals();
        m_unitSlots = BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed();
    }

    int minerals() {
        int res = m_minerals;
        for (auto type : pending_units) {
            res -= type.getMineralPrice();
        }
        return res;
    }

    int unitSlots() {
        int res = m_unitSlots;
        for (auto type : pending_units) {
            res += type.getSupplyProvided();
        }
        return res;
    }

    std::vector<Worker> m_workers;
    std::vector<Depot> m_depots;
    std::vector<Supply> m_supplies;
    std::vector<BWAPI::UnitType> pending_units;
    int m_minerals;
    int m_unitSlots;
};