
#include "AI.hpp"

class EventListener() {
	void onUnitCreate(BWAPI::Unit unit, BlackBoard& bb) {
		BWAPI::UnitType type = unit->getType();
		switch (type) {
			case BWAPI::UnitType::SUPPLY: {
				bb.m_supplies.push_back(Supply{unit, SupplyStates::UNKNOWN});
				break;
			}
			case BWAPI::UnitType::WORKER: {
				bb.m_workers.push_back(Worker{unit, WorkerStates::UNKNOWN});
				break;
			}
			case BWAPI::UnitType::DEPOT: {
				bb.m_depots.push_back(Depot{unit, DepotStates::UNKNOWN});
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
}