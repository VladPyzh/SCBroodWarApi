#pragma once

#include <map>
#include <vector>
#include <set>
#include <stdexcept>
#include <sstream>

#define WRITE_ENUM(out, var, val) \
if (var == val) {                 \
    out << #val;                  \
}

enum WorkerStates {
    W_UNKNOWN = 0,
    W_CREATING = 1,
    W_IDLE = 2,
    W_MINING = 3,
    W_GOING_TO_BUILD = 4,
    W_BUILDING = 5,
    W_IS_TO_RETURN_CARGO = 6,
    W_RETURNING_CARGO = 7,
    W_SCOUTING = 8,
    W_GASING = 9,
};

std::ostream& operator << (std::ostream& out, WorkerStates x) ;

enum DepotStates {
    D_UNKNOWN = 0,
    D_CREATING = 1,
    D_IDLE = 2,
    D_TRAINING = 3
};

std::ostream& operator << (std::ostream& out, DepotStates x);

enum RefineryStates {
    R_UNKNOWN = 0,
    R_CREATING = 1,
    R_IDLE = 2,
    R_EMPTY = 3
};

std::ostream& operator << (std::ostream& out, RefineryStates x);

enum SupplyStates {
    S_UNKNOWN = 0,
    S_CREATING = 1,
    S_IDLE = 2
};


std::ostream& operator << (std::ostream& out, SupplyStates x);

enum MarineStates {
    M_UNKNOWN = 0,
    M_CREATING = 1,
    M_IDLE = 2,
    M_MOVING = 3,
    M_PROTECTING = 4,
    M_ATTACKING = 5
};

std::ostream& operator << (std::ostream& out, MarineStates x);

enum MedicStates {
    Me_UNKNOWN = 0,
    Me_CREATING = 1,
    Me_IDLE = 2,
    Me_MOVING = 3,
    Me_HEALING = 4
};

std::ostream& operator << (std::ostream& out, MedicStates x) {
    WRITE_ENUM(out, x, Me_UNKNOWN);
    WRITE_ENUM(out, x, Me_CREATING);
    WRITE_ENUM(out, x, Me_IDLE);
    WRITE_ENUM(out, x, Me_MOVING);
    WRITE_ENUM(out, x, Me_HEALING);
    return out;
}

enum BarrackStates {
    B_UNKNOWN = 0,
    B_CREATING = 1,
    B_IDLE = 2,
    B_TRAINING = 3
};

std::ostream& operator << (std::ostream& out, BarrackStates x);

enum EnemyStates {
    E_UNKNOWN = 0,
    E_VISIBLE = 1
};

std::ostream& operator << (std::ostream& out, EnemyStates x);


enum AcademyStates {
    A_UNKNOWN = 0,
    A_CREATING = 1,
    A_IDLE = 2,
    A_UPGRADING = 3
};

std::ostream& operator << (std::ostream& out, AcademyStates x);

// State is a container for enum value of unit state.
// We use it in pair with FSM, see below
template <typename T>
struct State {
    State(): inner(T(0)) {}
    State(const State<T>& other) = delete;
    State& operator=(const State<T>& other) = delete;
    T inner;
};


// FSM is a finite state machine.
// It is used to keep track of units state and detect anomalies.
// States are updated either on ordering a new command (see Controller.hpp) or
// receiving new information from BWAPI (see BB::fetch and Event handler)
template <typename T>
struct FSM {
    FSM(std::map<T, std::set<T>> available): available(available) {}
    
    State<T> createState() {
        return State<T>(T());
    }
    void update(State<T>& cur, T next) {
        if (available[cur.inner].contains(next)) {
            cur.inner = next;
        } else {
            std::stringstream error_message;
            error_message << "invalid state transition: from " << cur.inner << " to " << next << '\n';
            throw std::runtime_error(error_message.str());
        }
    }
    std::map<T, std::set<T>> available;
};


const FSM<WorkerStates> WORKER_FSM({
    {W_UNKNOWN, {W_CREATING, W_IDLE}},
    {W_CREATING, {W_IDLE}},
    {W_IDLE, {W_MINING, W_GASING, W_GOING_TO_BUILD, W_IS_TO_RETURN_CARGO, W_SCOUTING}},
    {W_MINING, {W_IDLE, W_GOING_TO_BUILD, W_IS_TO_RETURN_CARGO}},
    {W_GOING_TO_BUILD, {W_BUILDING}},
    {W_BUILDING, {W_IDLE}},
    {W_IS_TO_RETURN_CARGO, {W_RETURNING_CARGO}},
    {W_RETURNING_CARGO, {W_IDLE}},
    {W_SCOUTING, {W_IDLE}},
    {W_GASING, {W_IDLE}}
});

const FSM<MarineStates> MARINE_FSM({
    {M_UNKNOWN, {M_CREATING, M_IDLE}},
    {M_CREATING, {M_IDLE}},
    {M_IDLE, {M_MOVING, M_PROTECTING, M_ATTACKING}},
    {M_MOVING, {M_PROTECTING, M_ATTACKING, M_IDLE}},
    {M_PROTECTING, {M_IDLE, M_MOVING}},
    {M_ATTACKING, {M_IDLE}}
 });

const FSM<MedicStates> MEDIC_FSM({
    {Me_UNKNOWN, {Me_CREATING, Me_IDLE}},
    {Me_CREATING, {Me_IDLE}},
    {Me_IDLE, {Me_MOVING, Me_HEALING}},
    {Me_MOVING, {Me_IDLE, Me_HEALING}},
    {Me_HEALING, {Me_IDLE, Me_MOVING}}
    });

const FSM<BarrackStates> BARRACK_FSM({
    {B_UNKNOWN, {B_CREATING, B_IDLE}},
    {B_CREATING, {B_IDLE}},
    {B_IDLE, {B_TRAINING}},
    {B_TRAINING, {B_IDLE}},
});

const FSM<DepotStates> DEPOT_FSM({
    {D_UNKNOWN, {D_CREATING, D_IDLE}},
    {D_CREATING, {D_IDLE}},
    {D_IDLE, {D_TRAINING}},
    {D_TRAINING, {D_IDLE}}
});

const FSM<RefineryStates> REFINERY_FSM({
    {R_UNKNOWN, {R_CREATING, R_IDLE}},
    {R_CREATING, {R_IDLE}},
    {R_IDLE, {R_EMPTY}}
 });

const FSM<SupplyStates> SUPPLY_FSM({
    {S_UNKNOWN, {S_CREATING, S_IDLE}},
    {S_CREATING, {S_IDLE}},
    {S_IDLE, {}}
});

const FSM<EnemyStates> ENEMY_FSM({
    {E_VISIBLE, {E_UNKNOWN}},
    {E_UNKNOWN, {E_VISIBLE}}
});

const FSM<AcademyStates> ACADEMY_FSM({
    {A_UNKNOWN, {A_CREATING, A_IDLE}},
    {A_CREATING, {A_IDLE}},
    {A_IDLE, {A_UPGRADING}},
    {A_UPGRADING, {A_IDLE}}
});

template<typename T>
FSM<T> provideFSM();

template<>
FSM<WorkerStates> provideFSM<WorkerStates>();
template<>
FSM<DepotStates> provideFSM<DepotStates>();
template<>
FSM<RefineryStates> provideFSM<RefineryStates>();
template<>
FSM<SupplyStates> provideFSM<SupplyStates>();
template<>
FSM<EnemyStates> provideFSM<EnemyStates>();
template<>
FSM<MarineStates> provideFSM<MarineStates>();
template<>
FSM<BarrackStates> provideFSM<BarrackStates>();
template<>
FSM<MedicStates> provideFSM<MedicStates>();
