#include <BWAPI.h>
#include <BWAPI/Client.h>
#include "MyAi/Ai.hpp"
#include "MyAi/EventHandler.hpp"
#include "ReplayParser.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>
void PlayGame();

int main(int argc, char * argv[])
{
    size_t gameCount = 0;

    // if we are not currently connected to BWAPI, try to reconnect
    while (!BWAPI::BWAPIClient.connect())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{ 1000 });
    }

    // if we have connected to BWAPI
    try {
        while (BWAPI::BWAPIClient.isConnected())
        {
            // the starcraft exe has connected but we need to wait for the game to start
            std::cout << "Waiting for game start\n";
            while (BWAPI::BWAPIClient.isConnected() && !BWAPI::Broodwar->isInGame())
            {
                BWAPI::BWAPIClient.update();
            }

            // Check to see if Starcraft shut down somehow
            if (BWAPI::BroodwarPtr == nullptr) { break; }

            // If we are successfully in a game, call the module to play the game
            if (BWAPI::Broodwar->isInGame())
            {
                if (!BWAPI::Broodwar->isReplay()) 
                { 
                    std::cout << "Something new added" << std::endl;
                    std::cout << "Playing Game " << gameCount++ << " on map " << BWAPI::Broodwar->mapFileName() << "\n";
                    PlayGame(); 
                }
            }
        }
    }
    catch (const std::runtime_error& e) {
        std::cout << e.what();
    }
    system("pause");
	return 0;
}

void PlayGame()
{
    AI ai;
    EventHandler handler;

    // The main game loop, which continues while we are connected to BWAPI and in a game
    while (BWAPI::BWAPIClient.isConnected() && BWAPI::Broodwar->isInGame())
    {
        // Handle each of the events that happened on this frame of the game
        for (const BWAPI::Event& e : BWAPI::Broodwar->getEvents())
        {
            switch (e.getType())
            {
                case BWAPI::EventType::MatchStart:   { ai.onStart();                       break; }
                case BWAPI::EventType::MatchFrame:   { ai.onFrame();                       break; }
                case BWAPI::EventType::MatchEnd:     { ai.onEnd(e.isWinner());             break; }
                case BWAPI::EventType::UnitCreate:   { handler.onUnitCreate(e.getUnit(), ai.blackBoard);       break; }
                case BWAPI::EventType::UnitComplete: { handler.onUnitComplete(e.getUnit(), ai.blackBoard);     break; }
                case BWAPI::EventType::UnitShow:     { handler.onUnitShow(e.getUnit(), ai.blackBoard);     break; }
                case BWAPI::EventType::UnitHide:     { handler.onUnitHide(e.getUnit(), ai.blackBoard);     break; }
            }
        }

        BWAPI::BWAPIClient.update();
        if (!BWAPI::BWAPIClient.isConnected())
        {
            std::cout << "Disconnected\n";
            break;
        }
    }

    std::cout << "Game Over\n";
}
