#include "BehaviorBase.hpp"


struct MoveOnRamp : public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(MoveOnRamp)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const;

    BWAPI::Position ramp_location = (BWAPI::Broodwar->self()->getStartLocation().x < 48) ? BWAPI::Position(54 * 32, 11 * 32) : BWAPI::Position(42 * 32, 117 * 32);

    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller);
};

