#include "BehaviorBase.hpp"


struct MoveOnRamp : public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(MoveOnRamp)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        auto unitsInRadius = BWAPI::Broodwar->getUnitsInRadius(ramp_location, 4 * 32);
        int cntMarines = 0;
        for (auto unit : unitsInRadius) {
            if (unit->getType() == BWAPI::UnitTypes::Terran_Marine) {
                cntMarines++;
            }
        }
        if (cntMarines < 7)  {
            return QuotaRequest{ 1, 7 - cntMarines, BWAPI::UnitTypes::Terran_Marine };
        } else {
            return QuotaRequest{ 1, 0, BWAPI::UnitTypes::Terran_Marine };
        }

    }

    BWAPI::Position ramp_location = (BWAPI::Broodwar->self()->getStartLocation().x < 48) ? BWAPI::Position(54 * 32, 11 * 32) : BWAPI::Position(42 * 32, 117 * 32);

    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller) {
        auto unitsInRadius = BWAPI::Broodwar->getUnitsInRadius(ramp_location, 20 * 32);
        int cntMarines = 0;
        for (auto unit : unitsInRadius) {
            if (unit->getType() == marine->unit->getType()) {
                cntMarines++;
            }
        }
        int distanceNum = cntMarines / 4 + 1;
        return
            bt::one_of({
                bt::if_true([marine, this, distanceNum]() {
                    return marine->unit->getPosition().getApproxDistance(ramp_location) < distanceNum * 32;
                }),
                bt::sequence({
                    bt::repeat_until_success([&controller, marine, &bb = std::as_const(bb), this]() {
                        return controller.moveUnit(marine, ramp_location);
                    }),
                    bt::repeat_node_until_success(bt::sequence({
                        bt::one_of({
                            bt::if_true([marine, this, distanceNum]() {
                                return marine->unit->getPosition().getApproxDistance(ramp_location) < distanceNum * 32;
                            }),
                            bt::if_true([marine](){
                                return marine->framesSinceUpdate > 200;
                            })
                        }),
                        bt::once([marine, &controller]() {
                            controller.stop(marine);
                        }),
                    }))
                }),
            });
    }
};

