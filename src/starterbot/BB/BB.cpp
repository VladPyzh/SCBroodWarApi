#include "BB.h"


void BlackBoard::update_baracks() {
	// BWAPI::Broodwar->printf("Updated barracks");
}

void BlackBoard::add_running_worker() {
	// BWAPI::Broodwar->printf("Added running worker");
}

void BlackBoard::update_blocked_money() {
    minerals_blocked = 0;
    gas_blocked = 0;
    for (auto& unit : BWAPI::Broodwar->self()->getUnits())
    {
        if (unit->getType().isWorker() && unit->isMoving() && unit->isConstructing())
        {
            BWAPI::UnitType buildingType = unit->getBuildType();
            minerals_blocked += buildingType.mineralPrice();
            gas_blocked += buildingType.gasPrice();
        }
    }
}