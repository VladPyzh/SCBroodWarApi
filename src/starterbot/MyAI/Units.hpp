
#pragma once
#include "FSM.hpp"
#include <BWAPI.h>
#include <memory>

template<typename T>
struct Unit {
    BWAPI::Unit unit;
    State<T> state;

    Unit(BWAPI::Unit unit): unit(unit), state() {}

    void changeState(T newState) {
        if (newState == state.inner) {
            return;
        }
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
