#pragma once
#include "BehaviorBase.hpp"

struct AttackBehavior: public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(AttackBehavior)

    Behavior::QuotaRequest submitQuotaRequest(const BlackBoard& bb) const;
    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller);
};

struct PushBehavior: public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(PushBehavior)

    Behavior::QuotaRequest submitQuotaRequest(const BlackBoard& bb) const ;
    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller) ;
};

struct SupportBehavior : public TreeBasedBehavior<MedicStates> {
    DECLARE_STR_TYPE(PushBehavior)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        int medics = bb.getUnits(MedicStates::Me_IDLE).size();

        return QuotaRequest{ 50, medics, BWAPI::UnitTypes::Terran_Medic };
        
    }
    std::shared_ptr<bt::node> createBT(Medic medic, const BlackBoard& bb, Controller& controller) {
        auto res = bt::one_of({
                bt::sequence({
                    bt::repeat_until_success([&controller, medic, &bb = std::as_const(bb), this]() {
                        return controller.moveUnit(medic, BWAPI::Position(48 * 32, 64 * 32));
                    })
                }),
                bt::fail()
            });
    }

};