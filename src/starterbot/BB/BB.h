#pragma once

#include <BWAPI.h>

class BlackBoard
{
	int running_workers;


public:
	int minerals_blocked;
	int gas_blocked;
	
	void update_baracks();
	void add_running_worker();
	void update_blocked_money();
};
