#pragma once

#include "BB.hpp"
#include "Units.hpp"
#include "../Tools.h"

/// Uses bwapi to do some actions
// ALong with issuing commands controller changes unit's state and
// verifies error code of corresponding BWAPI call
struct Controller {

    void addPendingUnit(const BlackBoard& bb, BWAPI::UnitType type);
    bool harvestMinerals(Worker worker) ;
    bool harvestGas(Worker worker) ;
    bool returnCargo(Worker worker) ;
    bool train(Depot depot, BWAPI::UnitType unitType, const BlackBoard& bb);
    bool train(Barrack barrack, BWAPI::UnitType unitType, const BlackBoard& bb) ;
    void stop(Marine marine);
    void moveUnit(Worker worker, BWAPI::Position targetPosition);
    bool moveUnit(Marine marine, BWAPI::Position targetPosition);
    bool build(Worker worker, BWAPI::UnitType buildingType, BWAPI::TilePosition buildPos, const BlackBoard& bb) ;
    bool attack(Marine marine, Enemy enemy) ;
    bool upgrade(Academy academy, BWAPI::UpgradeType upgradeType) ;
};