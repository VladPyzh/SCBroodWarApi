#pragma once
#include "BehaviorBase.hpp"

struct ScoutEnemyBaseBehaviour : public TreeBasedBehavior<WorkerStates> {
    DECLARE_STR_TYPE(ScoutEnemyBaseBehaviour)
   
    Behavior::QuotaRequest submitQuotaRequest(const BlackBoard& bb) const ;
    std::shared_ptr<bt::node> createBT(Worker worker, const BlackBoard& bb, Controller& controller);
    bool started = false;
};

struct LonelyScouting : public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(LonelyScouting)


    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const ;

    BWAPI::TilePosition findTile2Explore(Grid<int>when_seen, BWAPI::TilePosition my_pos, Marine marine) const;

    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller) ; 
};