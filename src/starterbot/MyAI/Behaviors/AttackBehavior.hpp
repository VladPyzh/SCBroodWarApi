#include "BehaviorBase.hpp"

struct AttackBehavior: public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(AttackBehavior)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const;
    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller);
};

struct PushBehavior: public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(PushBehavior)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const ;
    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller) ;
};
