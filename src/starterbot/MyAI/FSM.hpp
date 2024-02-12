#include <map>
#include <set>
#include <stdexcept>

enum WorkerStates {
    UNKNOWN = 0,
    CREATING = 1,
    IDLE = 2,
    MINING = 3,
    GOING_TO_BUILD = 4,
    BUILDING = 5,
    RETURNING_CARGO = 6
};

enum DepotStates {
    UNKNOWN = 0,
    CREATING = 1,
    IDLE = 2,
    TRAINING = 3
};

enum SupplyStates {
    UNKNOWN = 0,
    CREATING = 1,
    IDLE = 2
};

template <typename T>
struct State<T> {
    operator=(const State<T>& other) = delete;
    T inner;
};

template <typename T>
struct FSM<T> {
    FSM(std::map<T, std::vector<T>> available): available(available) {}
    
    State<T> createState() {
        return State<T>(T());
    }
    void update(State<T>& cur, T next) {
        if (available[cur.inner].contains(next)) {
            cur.inner = next;
        } else {
            throw std::runtime_error("invalid state transition");
        }
    }
    std::map<T, std::set<T>> available;
};


const FSM<WorkerStates> WORKER_FSM({
    {UNKNOWN, {CREATING}},
    {CREATING, {IDLE}},
    {IDLE, {MINING, GOING_TO_BUILD, RETURNING_CARGO}},
    {MINING, {IDLE, GOING_TO_BUILD, RETURNING_CARGO}},
    {GOING_TO_BUILD, {BUILDING}},
    {BUILDING, {IDLE}},
    {RETURNING_CARGO, {IDLE}}
});

const FSM<DepotStates> DEPOT_FSM({
    {UNKNOWN, {CREATING, IDLE}},
    {CREATING, {IDLE}},
    {IDLE, {TRAINING}},
    {TRAINING, {IDLE}}
});

const FSM<SupplyStates> SUPPLY_FSM({
    {UNKNOWN, {CREATING, IDLE}},
    {CREATING, {IDLE}},
    {IDLE, {}}
});


template<typename T>
FSM<T> provideFSM() { throw std::runtime_error("not defined"); }

FSM<WorkerStates> provideFSM() { return WORKER_FSM; }
FSM<DepotStates> provideFSM() { return DEPOT_FSM; }
FSM<SupplyStates> provideFSM() { return SUPPLY_FSM; }
