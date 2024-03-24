#include "UpgradingBehaviour.hpp"


Behavior::QuotaRequest UpgradeMarinesBehaviour::submitQuotaRequest(const BlackBoard& bb) const {
    if (!upgraded)
        return QuotaRequest{ 100, 1 - (int)trees.size(), BWAPI::UnitTypes::Terran_Academy };
    else
        return QuotaRequest{ 0, 0, BWAPI::UnitTypes::Terran_Academy };
}

std::shared_ptr<bt::node> UpgradeMarinesBehaviour::createBT(Academy academy, const BlackBoard& bb, Controller& controller) {
    return bt::sequence({
        bt::once([&controller, academy]() {
            controller->upgrade(academy, BWAPI::UpgradeTypes::U_238_Shells);
        }),
        bt::wait_until([academy]() {
            return depot->state.inner == A_IDLE;
        }),
        bt::once([this]() {
            upgraded = true;
        })
    });
}