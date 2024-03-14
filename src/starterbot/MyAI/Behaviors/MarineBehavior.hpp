#include "BehaviorBase.hpp"


struct MoveOnRamp : public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(MoveOnRamp)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        return QuotaRequest{ 100, 1, BWAPI::UnitTypes::Terran_Marine };
    }

    BWAPI::Position ramp_location = (BWAPI::Broodwar->self()->getStartLocation().x < 48) ? BWAPI::Position(54 * 32, 11 * 32) : BWAPI::Position(42 * 32, 117 * 32);

    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller) {
        return
            bt::one_of({
                bt::if_true([marine, this]() {
                    return marine->unit->getPosition().getApproxDistance(ramp_location) < 6 * 32;
                }),
                bt::sequence({
                    bt::repeat_until_success([&controller, marine, &bb = std::as_const(bb), this]() {
                        return controller.moveUnit(marine, ramp_location);
                    }),
                    bt::repeat_node_until_success(bt::one_of({
                        bt::if_true([marine]() {
                            return marine->state.inner == M_IDLE;
                        }),
                        bt::if_true([marine, this]() {
                            return marine->unit->getPosition().getApproxDistance(ramp_location) < 6 * 32;
                        }),
                    }))
                }),
            });
    }
};

