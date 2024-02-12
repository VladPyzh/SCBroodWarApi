#include "FSM.hpp"


template<typename T>
struct Unit<T> {
    BWAPI::Unit unit;
    State<T> state;

    void changeState(T newState) {
        if (newState == state.inner) {
            return;
        }
        provideFSM<T>().update(state, newState);
    }
};

typedef Unit<WorkerStates> Worker;
typedef Unit<DepotStates> Depot;
typedef Unit<SupplyStates> Supply;
