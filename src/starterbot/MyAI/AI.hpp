#pragma once

#include "BB.hpp"
#include "Controller.hpp"
#include "Planners.hpp"
#include <BWAPI.h>

struct AI
{
	BlackBoard blackBoard;
	Controller controller;
	Planner planner;

public:
    // functions that are triggered by various BWAPI events from main.cpp
	void onStart() {
		// Set our BWAPI options here    
		BWAPI::Broodwar->setLocalSpeed(1);
		BWAPI::Broodwar->setFrameSkip(0);
		
		// Enable the flag that tells BWAPI top let users enter input while bot plays
		BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);

		auto positions = BWAPI::Broodwar->getStartLocations();

		for (auto a : positions) {
			std::cerr << a.x << " " << a.y << std::endl;
		}

		std::cerr << BWAPI::Broodwar->mapWidth() << std::endl;

		blackBoard.init();
	}
	
	void onFrame() {
		blackBoard.fetch();
		planner.update(blackBoard, controller);
	}

	void onEnd(bool isWinner) {
		std::cout << "We " << (isWinner ? "won!" : "lost!") << "\n";
	}
};