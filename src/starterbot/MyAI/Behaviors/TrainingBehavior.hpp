#include "BehaviorBase.hpp"


struct TrainWorkersBehaviour : public TreeBasedBehavior<DepotStates> {
    DECLARE_STR_TYPE(TrainWorkersBehaviour)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const ;
    bool canTrainUnit(const BlackBoard& bb, BWAPI::UnitType type) ;
    std::shared_ptr<bt::node> createBT(Depot depot, const BlackBoard& bb, Controller& controller) ;
};

struct TrainMarinesBehaviour : public TreeBasedBehavior<BarrackStates> {
    DECLARE_STR_TYPE(TrainMarinesBehaviour)
    
    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const ;
    bool canTrainUnit(const BlackBoard& bb, BWAPI::UnitType type);
    std::shared_ptr<bt::node> createBT(Barrack barrack, const BlackBoard& bb, Controller& controller) ;
};
