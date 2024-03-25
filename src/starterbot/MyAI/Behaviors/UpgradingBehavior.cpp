#include "UpgradingBehavior.hpp"


Behavior::QuotaRequest UpgradeMarinesBehaviour::submitQuotaRequest(const BlackBoard& bb) const {
    if (!upgraded && canUpgrade(bb, BWAPI::UpgradeTypes::U_238_Shells)) {
        return QuotaRequest{ 100, 1 - (int)trees.size(), BWAPI::UnitTypes::Terran_Academy };
    }
    else {
        return QuotaRequest{ 0, 0, BWAPI::UnitTypes::Terran_Academy };
    }
}

bool UpgradeMarinesBehaviour::canUpgrade(const BlackBoard& bb, BWAPI::UpgradeType type) const {
    int minerals = bb.minerals();
    int gas = bb.gas();
    if (!bb.haveRefinery()) return false;
    return minerals >= type.mineralPrice(0) && gas >= type.gasPrice(0);
}

std::shared_ptr<bt::node> UpgradeMarinesBehaviour::createBT(Academy academy, const BlackBoard& bb, Controller& controller) {
    return bt::sequence({
        bt::wait_until([&bb = std::as_const(bb), this](){
            return canUpgrade(bb, BWAPI::UpgradeTypes::U_238_Shells);
        }),
        bt::once([&controller, academy]() {
            controller.upgrade(academy, BWAPI::UpgradeTypes::U_238_Shells);
        }),
        bt::wait_until([academy]() {
            return academy->state.inner == A_IDLE;
        }),
        bt::once([this]() {
            upgraded = true;
        })
    });
}