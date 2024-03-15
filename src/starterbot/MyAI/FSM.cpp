#include "FSM.hpp"

std::ostream& operator << (std::ostream& out, WorkerStates x) {
    WRITE_ENUM(out, x, W_UNKNOWN);
    WRITE_ENUM(out, x, W_CREATING);
    WRITE_ENUM(out, x, W_IDLE);
    WRITE_ENUM(out, x, W_MINING);
    WRITE_ENUM(out, x, W_GOING_TO_BUILD);
    WRITE_ENUM(out, x, W_BUILDING);
    WRITE_ENUM(out, x, W_IS_TO_RETURN_CARGO);
    WRITE_ENUM(out, x, W_RETURNING_CARGO);
    WRITE_ENUM(out, x, W_SCOUTING);
    WRITE_ENUM(out, x, W_GASING);

    return out;
}

std::ostream& operator << (std::ostream& out, DepotStates x) {
    WRITE_ENUM(out, x, D_UNKNOWN);
    WRITE_ENUM(out, x, D_CREATING);
    WRITE_ENUM(out, x, D_IDLE);
    WRITE_ENUM(out, x, D_TRAINING);
    return out;
}


std::ostream& operator << (std::ostream& out, RefineryStates x) {
    WRITE_ENUM(out, x, R_UNKNOWN);
    WRITE_ENUM(out, x, R_CREATING);
    WRITE_ENUM(out, x, R_IDLE);
    WRITE_ENUM(out, x, R_EMPTY);
    return out;
}



std::ostream& operator << (std::ostream& out, SupplyStates x) {
    WRITE_ENUM(out, x, S_UNKNOWN);
    WRITE_ENUM(out, x, S_CREATING);
    WRITE_ENUM(out, x, S_IDLE);
    return out;
}

std::ostream& operator << (std::ostream& out, MarineStates x) {
    WRITE_ENUM(out, x, M_UNKNOWN);
    WRITE_ENUM(out, x, M_CREATING);
    WRITE_ENUM(out, x, M_IDLE);
    WRITE_ENUM(out, x, M_MOVING);
    WRITE_ENUM(out, x, M_PROTECTING);
    WRITE_ENUM(out, x, M_ATTACKING);
    return out;
}

std::ostream& operator << (std::ostream& out, BarrackStates x) {
    WRITE_ENUM(out, x, B_UNKNOWN);
    WRITE_ENUM(out, x, B_CREATING);
    WRITE_ENUM(out, x, B_IDLE);
    WRITE_ENUM(out, x, B_TRAINING);
    return out;
}

std::ostream& operator << (std::ostream& out, EnemyStates x) {
    WRITE_ENUM(out, x, E_UNKNOWN);
    WRITE_ENUM(out, x, E_VISIBLE);
    return out;
}

std::ostream& operator << (std::ostream& out, AcademyStates x) {
    WRITE_ENUM(out, x, A_UNKNOWN);
    WRITE_ENUM(out, x, A_CREATING);
    WRITE_ENUM(out, x, A_IDLE);
    WRITE_ENUM(out, x, A_UPGRADING);
    return out;
}

template<typename T>
FSM<T> provideFSM() { throw std::runtime_error("not defined"); }

template<>
FSM<WorkerStates> provideFSM<WorkerStates>() { return WORKER_FSM; }
template<>
FSM<DepotStates> provideFSM<DepotStates>() { return DEPOT_FSM; }
template<>
FSM<RefineryStates> provideFSM<RefineryStates>() { return REFINERY_FSM; }
template<>
FSM<SupplyStates> provideFSM<SupplyStates>() { return SUPPLY_FSM; }
template<>
FSM<EnemyStates> provideFSM<EnemyStates>() { return ENEMY_FSM; }
template<>
FSM<MarineStates> provideFSM<MarineStates>() { return MARINE_FSM; }
template<>
FSM<BarrackStates> provideFSM<BarrackStates>() { return BARRACK_FSM; }
template<>
FSM<AcademyStates> provideFSM<AcademyStates>() { return ACADEMY_FSM; }
template<>
FSM<MedicStates> provideFSM<MedicStates>() { return MEDIC_FSM; }

