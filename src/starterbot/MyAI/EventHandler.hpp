#pragma once

#include "AI.hpp"
#include "BB.hpp"
#include <memory>

struct EventHandler {
	void onUnitCreate(BWAPI::Unit unit, BlackBoard& bb) {
		BWAPI::UnitType type = unit->getType();
		switch (type) {
		case BWAPI::UnitTypes::Terran_Supply_Depot: {
			bb.m_supplies.push_back(std::make_shared<SupplyUnit>(unit));
			break;
		}
		case BWAPI::UnitTypes::Terran_SCV: {
			bb.m_workers.push_back(std::make_shared<WorkerUnit>(unit));
			break;
		}
		case BWAPI::UnitTypes::Terran_Command_Center: {
			bb.m_depots.push_back(std::make_shared<DepotUnit>(unit));
			break;
		}
		case BWAPI::UnitTypes::Terran_Barracks: {
			bb.m_barracks.push_back(std::make_shared<BarrackUnit>(unit));
			break;
		}
		case BWAPI::UnitTypes::Terran_Marine: {
			bb.m_marines.push_back(std::make_shared<MarineUnit>(unit));
			break;
		}
		}
		for (int i = 0; i < bb.pending_units.size(); i++) {
			if (bb.pending_units[i] == type) {
				bb.pending_units.erase(bb.pending_units.begin() + i);
				break;
			}
		}
	}
	void onUnitComplete(BWAPI::Unit unit, BlackBoard& bb) {
		// update unit status
		
		// maybe we can fetch everything completed in bb itself????
	}
	void onUnitShow(BWAPI::Unit unit, BlackBoard& bb) {
		// update unit status
		/*
		if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()) == true) { // check if enemy
			bb.m_enemy.push_back(std::make_shared<VisibleEnemyUnit>(unit));
		}
		else {

		}
		*/
		// maybe we can fetch everything completed in bb itself????
	}
	void onUnitHide(BWAPI::Unit unit, BlackBoard& bb) {
		// update unit status

		// maybe we can fetch everything completed in bb itself????
	}
};