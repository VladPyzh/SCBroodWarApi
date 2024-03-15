#pragma once

#include "../MapTools.h"
#include "Units.hpp"
#include <vector>
#include <BWAPI.h>
#include <stdexcept>
#include "Log.hpp"

struct BlackBoard {
    void init();

    // the main source of incoming data for AI.
    // runcs on each frame and fetches updates 
    void fetch();

    // this block of methods keeps track of available resources with respect
    // to pending units
    int minerals() const;
    int gas() const;
    int freeUnitSlots() const;
    bool haveRefinery() const;
    int unitSlotsAvailable() const;

    BWAPI::UnitType workerType() const;
    BWAPI::UnitType marineType() const;
    BWAPI::UnitType medicType() const;
    
    // Units container management
    void removeUnit(int unitId);
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
    template<>
    std::vector<Academy> getUnits<AcademyStates>() const {
        return m_academy;
    }
    template<>
    std::vector<Medic> getUnits<MedicStates>() const {
        return m_medics;
    }

    // Stores map information (e.g. last seen info for tiles)
    MapTools m_mapTools;

    // Stores in-game available units
    std::vector<Medic> m_medics;
    std::vector<Worker> m_workers;
    std::vector<Depot> m_depots;
    std::vector<Supply> m_supplies;
    std::vector<Enemy> m_enemies;
    std::vector<Barrack> m_barracks;
    std::vector<Marine> m_marines;
    std::vector<Refinery> m_refineries;
    std::vector<Academy> m_academy;

    // Units that are not yet created but already issued
    // (e.g. Building that SCV just goes to construct)
    std::vector<BWAPI::UnitType> pending_units;
    // Resources available
    int m_minerals;
    int m_gas;
    int m_unitSlotsAvailable;
    int m_unitSlotsTaken;
};