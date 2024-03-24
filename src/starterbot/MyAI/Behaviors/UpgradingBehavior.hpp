#include "BehaviorBase.hpp"


struct UpgradeMarinesBehaviour : public TreeBasedBehavior<AcademyStates> {
    DECLARE_STR_TYPE(UpgradeMarinesBehaviour)

    Behavior::QuotaRequest submitQuotaRequest(const BlackBoard& bb) const ;
    std::shared_ptr<bt::node> createBT(Depot depot, const BlackBoard& bb, Controller& controller) ;

    bool upgraded = false;
};

