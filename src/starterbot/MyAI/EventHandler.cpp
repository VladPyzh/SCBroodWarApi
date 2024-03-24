#include "EventHandler.hpp"

void EventHandler::onUnitCreate(BWAPI::Unit unit, BlackBoard& bb) {
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

void EventHandler::onUnitMorph(BWAPI::Unit unit, BlackBoard& bb) {
	BWAPI::UnitType type = unit->getType();
	switch (type) {
	case BWAPI::UnitTypes::Terran_Refinery: {
		bb.m_refineries.push_back(std::make_shared<RefineryUnit>(unit));
		break;
	}
	}
	for (int i = 0; i < bb.pending_units.size(); i++) { // NOT SURE FOR ALL? CAN ERASE TANKS
		if (bb.pending_units[i] == type) {
			bb.pending_units.erase(bb.pending_units.begin() + i);
			break;
		}
	}
}


void EventHandler::onUnitComplete(BWAPI::Unit unit, BlackBoard& bb) {
	
}

void EventHandler::onDiscovery(BWAPI::Unit unit, BlackBoard& bb) {
	if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()) == true) { // check if enemy
		bb.m_enemies.push_back(std::make_shared<EnemyUnit>(unit));
	}
}
/*
void onUnitShow(BWAPI::Unit unit, BlackBoard& bb) {
	if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()) == true) { // check if enemy
		bb.m_enemies.push_back(std::make_shared<EnemyUnit>(unit));
	}
}
*/
void EventHandler::onUnitHide(BWAPI::Unit unit, BlackBoard& bb) {
	if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()) == true) { // check if enemy
		bb.removeUnit(unit->getID());
	}
}
void EventHandler::onUnitDestroyed(BWAPI::Unit unit, BlackBoard& bb) {
	bb.removeUnit(unit->getID());
}