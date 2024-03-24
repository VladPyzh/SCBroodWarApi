#pragma once

#include "BB.hpp"
#include <memory>

// EventHandler is responsible for all external updates from BWAPI
struct EventHandler {
	void onUnitCreate(BWAPI::Unit unit, BlackBoard& bb);
	void onUnitMorph(BWAPI::Unit unit, BlackBoard& bb);
	void onUnitComplete(BWAPI::Unit unit, BlackBoard& bb);
	void onDiscovery(BWAPI::Unit unit, BlackBoard& bb);
	void onUnitHide(BWAPI::Unit unit, BlackBoard& bb);
	void onUnitDestroyed(BWAPI::Unit unit, BlackBoard& bb);
};