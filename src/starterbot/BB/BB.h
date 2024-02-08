#pragma once

#include <BWAPI.h>

class BlackBoard
{
	int running_workers;
	int minerals_blocked;
	int gas_blocked;

	int marine_captain = -1;

public:

	BlackBoard();
	int get_blocked_minerals() { return minerals_blocked; }
	int get_marine_captain();

	void update_marine_captain();
	void update_baracks();
	void add_running_worker();
	void update_blocked_money();
};
