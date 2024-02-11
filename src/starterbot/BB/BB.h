#pragma once

#include <BWAPI.h>

enum WorkersAssigment {
	UNKNOWN, // consider dead or didn't created
	CREATING,
	IDLE,
	MINNING,
	GOING_TO_BUILDING,
	BUILDING,
	HIDDING,
	RETURNING_CARGO
};

class BlackBoard
{
public:

	std::vector<WorkersAssigment> workers;
	std::vector<int> worker_ids;

	std::vector<std::pair<BWAPI::UnitType, int>> planning; // type of building, who is building
	std::vector<std::pair<BWAPI::Unit, int>> building; // type of building, who is building


	BlackBoard();

	void print();

	int getNumberOfWorkers();
	bool isWorker(BWAPI::Unit unit);
	void addPlanningBuilding(BWAPI::UnitType supplyType, int builder);
	void addBuildingBuilding(BWAPI::Unit unitToBuild, int builder);
	bool needSupply();
	void addUnit(BWAPI::Unit unit);
	BWAPI::Unit getWorkerById(int id);
	BWAPI::Unit getIdleWorker();
	BWAPI::Unit getCargoWorker();
	BWAPI::Unit getWorkerForBuilding();
	void updateWorker(int worker_id, WorkersAssigment assigment);

};
