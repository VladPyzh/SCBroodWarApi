#pragma once

#include "BB.hpp"
#include "Controller.hpp"
#include "Planners.hpp"
#include "EventHandler.hpp"
#include <BWAPI.h>

// Refer to README.md for general architecture
struct AI
{
	BlackBoard blackBoard;
	Controller controller;
	Planner planner;
	EventHandler eventHandler;

public:
    // functions that are triggered by various BWAPI events from main.cpp
	void onStart();
	void onFrame();
	void onEvent(const BWAPI::Event& e);
	void onEnd(bool isWinner);
};