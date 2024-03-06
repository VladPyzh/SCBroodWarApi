#include "BehaviorBase.hpp"


struct MoveOnRamp : public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(MoveOnRamp)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        return QuotaRequest{ 100, 1, BWAPI::UnitTypes::Terran_Marine };
    }

    BWAPI::Position ramp_location = BWAPI::Position(54 * 32, 11 * 32);

    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller) {
        return bt::sequence({
            bt::repeat_until_success([&controller, marine, &bb = std::as_const(bb), this]() {
                return controller.moveUnit(marine, ramp_location);
            }),
            bt::wait_until([marine]() {
                return marine->state.inner == M_IDLE;
            })
        });
    }
};

