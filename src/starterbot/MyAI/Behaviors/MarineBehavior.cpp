#include "MarineBehavior.hpp"



QuotaRequest MoveOnRamp::submitQuotaRequest(const BlackBoard& bb) const {
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


std::shared_ptr<bt::node> MoveOnRamp::createBT(Marine marine, const BlackBoard& bb, Controller& controller) {
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

