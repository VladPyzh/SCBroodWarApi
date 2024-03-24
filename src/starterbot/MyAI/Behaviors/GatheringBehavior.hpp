#pragma once
#include "BehaviorBase.hpp"

struct GatherMineralsBehavior: public TreeBasedBehavior<WorkerStates> {
    DECLARE_STR_TYPE(GatherMineralsBehavior)

    Behavior::QuotaRequest submitQuotaRequest(const BlackBoard& bb) const ;
    std::shared_ptr<bt::node> createBT(Worker worker, const BlackBoard& bb, Controller& controller);
};

struct GatherGasBehavior : public TreeBasedBehavior<WorkerStates> {
    DECLARE_STR_TYPE(GatherGasBehavior)

    Behavior::QuotaRequest submitQuotaRequest(const BlackBoard& bb) const ;
    std::shared_ptr<bt::node> createBT(Worker worker, const BlackBoard& bb, Controller& controller) ;
};
