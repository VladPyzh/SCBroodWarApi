
#pragma once
#include "FSM.hpp"
#include "Log.hpp"
#include <BWAPI.h>
#include <memory>

constexpr bool DEBUG_STATE_TRANSITIONS = true;

struct BaseUnit {
    virtual ~BaseUnit() {}
};

template<typename T>
struct Unit : BaseUnit {
    BWAPI::Unit unit;
    State<T> state;
    bool isActive = false;

    Unit(BWAPI::Unit unit): unit(unit), state() {}

    void changeState(T newState) {
        if (newState == state.inner) {
            return;
        }
        DEBUG_LOG(DEBUG_STATE_TRANSITIONS, unit->getID() << "from state " << state.inner << " to state " << newState << std::endl)
        provideFSM<T>().update(state, newState);
    }
};

typedef Unit<WorkerStates> WorkerUnit;
typedef Unit<DepotStates> DepotUnit;
typedef Unit<SupplyStates> SupplyUnit;
typedef Unit<MarineStates> MarineUnit;
typedef Unit<BarrackStates> BarrackUnit;

//typedef Unit<VisibleEnemyStates> VisibleEnemyUnit;


typedef std::shared_ptr<WorkerUnit> Worker;
typedef std::shared_ptr<DepotUnit> Depot;
typedef std::shared_ptr<SupplyUnit> Supply;
typedef std::shared_ptr<MarineUnit> Marine;
typedef std::shared_ptr<BarrackUnit> Barrack;


//typedef std::shared_ptr<VisibleEnemyUnit> VisibleEnemy;
