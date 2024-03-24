#include "BehaviorBase.hpp"

struct GatherMineralsBehavior: public TreeBasedBehavior<WorkerStates> {
    DECLARE_STR_TYPE(GatherMineralsBehavior)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const ;
    std::shared_ptr<bt::node> createBT(Worker worker, const BlackBoard& bb, Controller& controller);
};

struct GatherGasBehavior : public TreeBasedBehavior<WorkerStates> {
    DECLARE_STR_TYPE(GatherGasBehavior)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const ;
    std::shared_ptr<bt::node> createBT(Worker worker, const BlackBoard& bb, Controller& controller) ;
};
