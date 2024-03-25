#include "BehaviorBase.hpp"


struct UpgradeMarinesBehaviour : public TreeBasedBehavior<AcademyStates> {
    DECLARE_STR_TYPE(UpgradeMarinesBehaviour)

    Behavior::QuotaRequest submitQuotaRequest(const BlackBoard& bb) const ;
    bool canUpgrade(const BlackBoard& bb, BWAPI::UpgradeType type) const;
    std::shared_ptr<bt::node> createBT(Academy academy, const BlackBoard& bb, Controller& controller) ;

    bool upgraded = false;
};

