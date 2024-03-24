#pragma once
#include "BehaviorBase.hpp"


BWAPI::TilePosition findClosestVespeneGeyser(BWAPI::Unit worker);

struct ConstructingBehavior : public TreeBasedBehavior<WorkerStates> {
    DECLARE_STR_TYPE(ConstructingBehavior)

    int cur_build_index = 0;
    std::vector<BWAPI::UnitType> build_order = {
      BWAPI::UnitTypes::Terran_Supply_Depot,
      BWAPI::UnitTypes::Terran_Barracks,
      BWAPI::UnitTypes::Terran_Supply_Depot,
      BWAPI::UnitTypes::Terran_Refinery,
      BWAPI::UnitTypes::Terran_Barracks,
      BWAPI::UnitTypes::Terran_Academy,
      BWAPI::UnitTypes::Terran_Barracks,
      BWAPI::UnitTypes::Terran_Supply_Depot,
      BWAPI::UnitTypes::Terran_Barracks,
    };

    std::vector<BWAPI::TilePosition> positions = {
        (BWAPI::Broodwar->self()->getStartLocation().x < 48) ? BWAPI::TilePosition(49, 7) : BWAPI::TilePosition(46, 120),
        (BWAPI::Broodwar->self()->getStartLocation().x < 48) ? BWAPI::TilePosition(46, 11) : BWAPI::TilePosition(46, 114),
        (BWAPI::Broodwar->self()->getStartLocation().x < 48) ? BWAPI::TilePosition(46, 7) : BWAPI::TilePosition(50, 120),
        BWAPI::TilePosition(0, 0),
        (BWAPI::Broodwar->self()->getStartLocation().x < 48) ? BWAPI::TilePosition(42, 11) : BWAPI::TilePosition(50, 114),
        (BWAPI::Broodwar->self()->getStartLocation().x < 48) ? BWAPI::TilePosition(37, 4) : BWAPI::TilePosition(54, 114), // fix
        (BWAPI::Broodwar->self()->getStartLocation().x < 48) ? BWAPI::TilePosition(37, 9) : BWAPI::TilePosition(55, 112),
        (BWAPI::Broodwar->self()->getStartLocation().x < 48) ? BWAPI::TilePosition(37, 13) : BWAPI::TilePosition(55, 115),

                /*
        BWAPI::TilePosition(46, 11),
        BWAPI::TilePosition(46, 7),
        BWAPI::TilePosition(0, 0),
        BWAPI::TilePosition(42, 11),
        BWAPI::TilePosition(38, 11),
        */
    };

    bool canConstruct(const BlackBoard& bb, BWAPI::UnitType type) const;
    bool shouldConstruct(const BlackBoard& bb, BWAPI::UnitType type) const;

    Behavior::QuotaRequest submitQuotaRequest(const BlackBoard& bb) const;

    std::shared_ptr<bt::node> createBT(Worker worker, const BlackBoard& bb, Controller& controller);
};
