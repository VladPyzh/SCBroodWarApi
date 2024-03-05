#include "BehaviorBase.hpp"


struct TrainWorkersBehaviour : public TreeBasedBehavior<DepotStates> {
    DECLARE_STR_TYPE(TrainWorkersBehaviour)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        return QuotaRequest{ 100, 1 - (int)trees.size(), BWAPI::UnitTypes::Terran_Supply_Depot };
    }
    bool canTrainUnit(const BlackBoard& bb, BWAPI::UnitType type) {
        int minerals = bb.minerals();
        int unitSlots = bb.freeUnitSlots();
        return minerals >= type.mineralPrice() && unitSlots >= type.supplyRequired();
    }
    std::shared_ptr<bt::node> createBT(Depot depot, const BlackBoard& bb, Controller& controller) {
        return bt::sequence({
            bt::wait_until([&bb = std::as_const(bb), this]() {
                return canTrainUnit(bb, bb.workerType());
            }),
            bt::repeat_until_success([&controller, depot, &bb = std::as_const(bb)]() {
                return controller.train(depot, bb.workerType(), bb);
            }),
            bt::wait_until([depot]() {
                return depot->state.inner == D_IDLE;
            })
        });
    }
};

struct TrainMarinesBehaviour : public TreeBasedBehavior<BarrackStates> {
    DECLARE_STR_TYPE(TrainMarinesBehaviour)
    
    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        return QuotaRequest{ 100, 1 - (int)trees.size(), BWAPI::UnitTypes::Terran_Barracks };
    }
    bool canTrainUnit(const BlackBoard& bb, BWAPI::UnitType type) {
        int minerals = bb.minerals();
        int unitSlots = bb.freeUnitSlots();
        return minerals >= type.mineralPrice() && unitSlots >= type.supplyRequired();
    }
    std::shared_ptr<bt::node> createBT(Barrack barrack, const BlackBoard& bb, Controller& controller) {
        return bt::sequence({
            bt::wait_until([&bb = std::as_const(bb), this]() {
                return canTrainUnit(bb, bb.marineType());
            }),
            bt::repeat_until_success([&controller, barrack, &bb = std::as_const(bb)]() {
                return controller.train(barrack, bb.marineType(), bb);
            }),
            bt::wait_until([barrack]() {
                return barrack->state.inner == B_IDLE;
            })
        });
    }
};
