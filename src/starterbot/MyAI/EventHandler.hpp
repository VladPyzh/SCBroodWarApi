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
};