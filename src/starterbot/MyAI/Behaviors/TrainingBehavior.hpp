#include "BehaviorBase.hpp"


struct TrainWorkersBehaviour : public TreeBasedBehavior<DepotStates> {
    DECLARE_STR_TYPE(TrainWorkersBehaviour)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        return QuotaRequest{ 100, 1 - (int)trees.size(), BWAPI::UnitTypes::Terran_Supply_Depot };
    }
    bool canTrainUnit(const BlackBoard& bb, BWAPI::UnitType type) {
        int minerals = bb.minerals();
        int unitSlots = bb.freeUnitSlots();
        return minerals >= type.mineralPrice() && unitSlots >= type.supplyRequired() && bb.m_workers.size() < 18 + 3;
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
    
    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {                                          // for medics ---\/
        return QuotaRequest{ 100, std::max(0, (int)bb.getUnits<BarrackStates>().size() - (int)trees.size() - bb.haveRefinery()), BWAPI::UnitTypes::Terran_Barracks }; // MIND QUOTA!!
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

struct TrainMedicsBehaviour : public TreeBasedBehavior<BarrackStates> {
    DECLARE_STR_TYPE(TrainMedicsBehaviour)

    bool canTrainUnit(const BlackBoard& bb, BWAPI::UnitType type) const {
        int minerals = bb.minerals();
        int gas = bb.gas();
        int unitSlots = bb.freeUnitSlots();
        return minerals >= type.mineralPrice() && gas >= type.gasPrice() && unitSlots >= type.supplyRequired();
    }

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        if (canTrainUnit(bb, bb.medicType()) && bb.haveRefinery()) {
            return QuotaRequest{ 100, std::max(0, 1 - (int)trees.size()), BWAPI::UnitTypes::Terran_Barracks }; // MIND QUOTA!!
        }
        else {
            return QuotaRequest{ 100, 0, BWAPI::UnitTypes::Terran_Barracks }; // MIND QUOTA!!
        }
        
    }
    
    std::shared_ptr<bt::node> createBT(Barrack barrack, const BlackBoard& bb, Controller& controller) {
        return bt::sequence({
            bt::wait_until([&bb = std::as_const(bb), this]() {
                return canTrainUnit(bb, bb.medicType());
            }),
            bt::repeat_until_success([&controller, barrack, &bb = std::as_const(bb)]() {
                return controller.train(barrack, bb.medicType(), bb);
            }),
            bt::wait_until([barrack]() {
                return barrack->state.inner == B_IDLE;
            })
            });
    }
};
