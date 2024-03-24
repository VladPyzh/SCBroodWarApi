#include "AI.hpp"

void AI::onStart() {
	// Set our BWAPI options here    
	BWAPI::Broodwar->setLocalSpeed(2);
	BWAPI::Broodwar->setFrameSkip(0);
	
	// Enable the flag that tells BWAPI top let users enter input while bot plays
	BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);
	std::cerr << "FLAGS IS " << BWAPI::Broodwar->isFlagEnabled(BWAPI::Flag::CompleteMapInformation) << std::endl;
	blackBoard.init();
}

void AI::onEvent(const BWAPI::Event& e) {
	switch (e.getType()) {
		case BWAPI::EventType::UnitCreate:   { eventHandler.onUnitCreate(e.getUnit(), blackBoard);       break; }
		case BWAPI::EventType::UnitComplete: { eventHandler.onUnitComplete(e.getUnit(), blackBoard);     break; }
		case BWAPI::EventType::UnitDestroy:  { eventHandler.onUnitDestroyed(e.getUnit(), blackBoard);     break; }
		case BWAPI::EventType::UnitDiscover: { eventHandler.onDiscovery(e.getUnit(), blackBoard);     break; }
		case BWAPI::EventType::UnitHide:     { eventHandler.onUnitHide(e.getUnit(), blackBoard);     break; }
		case BWAPI::EventType::UnitRenegade: { eventHandler.onUnitHide(e.getUnit(), blackBoard);     break; }
		case BWAPI::EventType::UnitMorph:    { eventHandler.onUnitMorph(e.getUnit(), blackBoard);     break; }
		//case BWAPI::EventType::UnitShow:     { handler.onUnitShow(e.getUnit(), ai.blackBoard);     break; }
	}
}

	
void AI::onFrame() {
	blackBoard.fetch();
	planner.update(blackBoard, controller);
}

void AI::onEnd(bool isWinner) {
	std::cout << "We " << (isWinner ? "won!" : "lost!") << "\n";
}
