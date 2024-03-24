
#pragma once
#include "FSM.hpp"
#include "Log.hpp"
#include <BWAPI.h>
#include <memory>

constexpr bool DEBUG_STATE_TRANSITIONS = false;

struct BaseUnit {
    virtual ~BaseUnit() {}
};

template<typename T>
struct Unit : BaseUnit {
    // BWAPI unit object
    BWAPI::Unit unit;
    // Strongly verifyed state of current unit, see FSM.hpp for more details
    State<T> state;
    // we calculate number of frames since last state update, because sometimes BWAPI
    // provides data with a small lag
    int framesSinceUpdate = 0;
    // determines if unit has a behavior attached or not
    bool isActive = false;
    // Service variable for debug purposes
    bool isHighLighted = false;

    Unit(BWAPI::Unit unit): unit(unit), state() {}


    // Changes unit state. Verifies that state transition is defined as available
    void changeState(T newState) {
        if (newState == state.inner) {
            return;
        }
        framesSinceUpdate = 0;
        DEBUG_LOG(DEBUG_STATE_TRANSITIONS, unit->getID() << "from state " << state.inner << " to state " << newState << std::endl)
        provideFSM<T>().update(state, newState);
    }

    // Draws a box over unit
    void highlight() {
        if (isHighLighted) {
            BWAPI::Position topLeft(unit->getLeft(), unit->getTop());
            BWAPI::Position bottomRight(unit->getRight(), unit->getBottom());
            BWAPI::Broodwar->drawBoxMap(topLeft, bottomRight, BWAPI::Colors::White);
        }
    }
};

typedef Unit<WorkerStates> WorkerUnit;
typedef Unit<DepotStates> DepotUnit;
typedef Unit<RefineryStates> RefineryUnit;
typedef Unit<SupplyStates> SupplyUnit;
typedef Unit<MarineStates> MarineUnit;
typedef Unit<BarrackStates> BarrackUnit;
typedef Unit<MedicStates> MedicUnit;

typedef Unit<EnemyStates> EnemyUnit;
typedef Unit<AcademyStates> AcademyUnit;

//typedef Unit<VisibleEnemyStates> VisibleEnemyUnit;


typedef std::shared_ptr<WorkerUnit> Worker;
typedef std::shared_ptr<DepotUnit> Depot;
typedef std::shared_ptr<RefineryUnit> Refinery;
typedef std::shared_ptr<SupplyUnit> Supply;
typedef std::shared_ptr<MarineUnit> Marine;
typedef std::shared_ptr<BarrackUnit> Barrack;
typedef std::shared_ptr<MedicUnit> Medic;

typedef std::shared_ptr<EnemyUnit> Enemy;
typedef std::shared_ptr<AcademyUnit> Academy;


//typedef std::shared_ptr<VisibleEnemyUnit> VisibleEnemy;
