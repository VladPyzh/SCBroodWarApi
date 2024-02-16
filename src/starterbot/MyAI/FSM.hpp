#pragma once

#include <map>
#include <vector>
#include <set>
#include <stdexcept>
#include <sstream>



enum WorkerStates {
    W_UNKNOWN = 0,
    W_CREATING = 1,
    W_IDLE = 2,
    W_MINING = 3,
    W_GOING_TO_BUILD = 4,
    W_BUILDING = 5,
    W_RETURNING_CARGO = 6
};

#define WRITE_ENUM(out, var, val) \
if (var == val) {                 \
    out << #val;                  \
}

std::ostream& operator << (std::ostream& out, WorkerStates x) {
    WRITE_ENUM(out, x, W_UNKNOWN);
    WRITE_ENUM(out, x, W_CREATING);
    WRITE_ENUM(out, x, W_IDLE);
    WRITE_ENUM(out, x, W_MINING);
    WRITE_ENUM(out, x, W_GOING_TO_BUILD);
    WRITE_ENUM(out, x, W_BUILDING);
    WRITE_ENUM(out, x, W_RETURNING_CARGO);
    return out;
}

enum DepotStates {
    D_UNKNOWN = 0,
    D_CREATING = 1,
    D_IDLE = 2,
    D_TRAINING = 3
};

std::ostream& operator << (std::ostream& out, DepotStates x) {
    WRITE_ENUM(out, x, D_UNKNOWN);
    WRITE_ENUM(out, x, D_CREATING);
    WRITE_ENUM(out, x, D_IDLE);
    WRITE_ENUM(out, x, D_TRAINING);
    return out;
}

enum SupplyStates {
    S_UNKNOWN = 0,
    S_CREATING = 1,
    S_IDLE = 2
};


std::ostream& operator << (std::ostream& out, SupplyStates x) {
    WRITE_ENUM(out, x, S_UNKNOWN);
    WRITE_ENUM(out, x, S_CREATING);
    WRITE_ENUM(out, x, S_IDLE);
    return out;
}

template <typename T>
struct State {
    State(): inner(T(0)) {}
    State(const State<T>& other) = delete;
    State& operator=(const State<T>& other) = delete;
    T inner;
};

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
    {W_IDLE, {W_MINING, W_GOING_TO_BUILD, W_RETURNING_CARGO}},
    {W_MINING, {W_IDLE, W_GOING_TO_BUILD, W_RETURNING_CARGO}},
    {W_GOING_TO_BUILD, {W_BUILDING}},
    {W_BUILDING, {W_IDLE}},
    {W_RETURNING_CARGO, {W_IDLE}}
});

const FSM<DepotStates> DEPOT_FSM({
    {D_UNKNOWN, {D_CREATING, D_IDLE}},
    {D_CREATING, {D_IDLE}},
    {D_IDLE, {D_TRAINING}},
    {D_TRAINING, {D_IDLE}}
});

const FSM<SupplyStates> SUPPLY_FSM({
    {S_UNKNOWN, {S_CREATING, S_IDLE}},
    {S_CREATING, {S_IDLE}},
    {S_IDLE, {}}
});


template<typename T>
FSM<T> provideFSM() { throw std::runtime_error("not defined"); }

template<>
FSM<WorkerStates> provideFSM<WorkerStates>() { return WORKER_FSM; }
template<>
FSM<DepotStates> provideFSM<DepotStates>() { return DEPOT_FSM; }
template<>
FSM<SupplyStates> provideFSM<SupplyStates>() { return SUPPLY_FSM; }
