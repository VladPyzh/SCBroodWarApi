#include "BB.h"
#include <iostream>

BlackBoard::BlackBoard() {

}

void BlackBoard::print() {
  // std::cout << "===\n";
  std::cout << "Blackboard(";
  for (int i = 0; i < workers.size(); i++) {
  	if (workers[i] != WorkersAssigment::MINNING)
	    std::cout << '\t' << worker_ids[i] << ' ' << workers[i];
  }
  std::cout << ")\r";
}

int BlackBoard::getNumberOfWorkers() // be aware we count only known workers
{
	int count = 0;
	for (int i = 0; i < workers.size(); ++i) {
		if (workers[i] != WorkersAssigment::UNKNOWN) {
			++count;
		}
	}
	return count;
}

bool BlackBoard::isWorker(BWAPI::Unit unit)
{
	for (int i = 0; i < worker_ids.size(); ++i) {
		if (worker_ids[i] == unit->getID()) {
			return true;
		}
	}
	return false;
}

void BlackBoard::addPlanningBuilding(BWAPI::UnitType supplyType, int builder) {
	planning.push_back({ supplyType, builder });
}

void BlackBoard::addBuildingBuilding(BWAPI::Unit unitToBuild, int builder) {
	int idx = -1;
	for (int i = 0; i < planning.size(); i++) {
		if (planning[i].second == builder) {
			idx = i;
			break;
		}
	}
	if (idx != -1) {
		planning.erase(planning.begin() + idx);
	}
	building.push_back({ unitToBuild, builder });
}

bool BlackBoard::needSupply() {
	const int current_available_supply = BWAPI::Broodwar->self()->supplyTotal();
	int building_supply = 0;
	for (int i = 0; i < building.size(); ++i) {
		building_supply += building[i].first->getType().supplyProvided();
	}
	int planning_supply = 0;
	for (int i = 0; i < planning.size(); ++i) {
		planning_supply += planning[i].first.supplyProvided();
	}

	if ((current_available_supply + building_supply + planning_supply - BWAPI::Broodwar->self()->supplyUsed() < 3) && (BWAPI::Broodwar->self()->minerals() > 100)) {
		return true;
	}
	else {
		return false;
	}
}

void BlackBoard::addUnit(BWAPI::Unit unit) {
	BWAPI::UnitType type = unit->getType();
	if (type.isWorker()) {
		workers.push_back(WorkersAssigment::CREATING);
		worker_ids.push_back(unit->getID());
	}
	else {
		BWAPI::Broodwar->printf("Added %s unit type", unit->getType().getName());
	}
}

BWAPI::Unit BlackBoard::getWorkerById(int id) {
	for (auto& unit : BWAPI::Broodwar->self()->getUnits()) {
		if (unit->getType().isWorker() && unit->getID() == id)
		{
			return unit;
		}
	}
	return nullptr;
}

BWAPI::Unit BlackBoard::getIdleWorker()
{
	for (int i = 0; i < workers.size(); ++i) {
		if (workers[i] == WorkersAssigment::IDLE) {
			return getWorkerById(worker_ids[i]);
		}
	}
	return nullptr;
}

BWAPI::Unit BlackBoard::getCargoWorker()
{
	for (int i = 0; i < workers.size(); ++i) {
		if (workers[i] == WorkersAssigment::RETURNING_CARGO) {
			return getWorkerById(worker_ids[i]);
		}
	}
	return nullptr;
}

BWAPI::Unit BlackBoard::getWorkerForBuilding()
{
	for (int i = 0; i < workers.size(); ++i) {
		if (workers[i] == WorkersAssigment::MINNING) {
			return getWorkerById(worker_ids[i]);
		}
	}
	return nullptr;
}

void BlackBoard::updateWorker(int worker_id, WorkersAssigment assigment) {
	std::cout << "SCV " << worker_id << " changed to " << assigment << std::endl;
	for (int i = 0; i < worker_ids.size(); ++i) {
		if (worker_ids[i] == worker_id) {
			workers[i] = assigment;
			break;
		}
	}
}
