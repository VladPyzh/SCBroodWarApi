#include "StarterBot.h"
#include "Tools.h"
#include "MapTools.h"
#include "BB/BB.h"

StarterBot::StarterBot()
{

}

// Called when the bot starts!
void StarterBot::onStart()
{
    // Set our BWAPI options here    
	BWAPI::Broodwar->setLocalSpeed(10);
    BWAPI::Broodwar->setFrameSkip(0);
    
    // Enable the flag that tells BWAPI top let users enter input while bot plays
    BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);

    // Call MapTools OnStart
    m_mapTools.onStart();
}

void StarterBot::onMenuFrame()
{
    print = false;
}

// Called on each frame of the game
void StarterBot::onFrame()
{
    // Update our MapTools information
    m_mapTools.onFrame();

    if (print) {
        blackBoard.print();
    }
    // Send our idle workers to mine minerals so they don't just stand there
    updateReturningCargo();

    sendIdleWorkersToMinerals();

    // Train more workers so we can gather more income
    trainAdditionalWorkers();

    // Build more supply if we are going to run out soon
    if (blackBoard.needSupply()) {
        buildAdditionalSupply();
    }

    // Draw unit health bars, which brood war unfortunately does not do
    Tools::DrawUnitHealthBars();

    // Draw some relevent information to the screen to help us debug the bot
    drawDebugInformation();
}

void StarterBot::updateReturningCargo() {
    BWAPI::Unit worker = blackBoard.getCargoWorker();
    if (worker == nullptr || worker->isCarryingMinerals()) {
        return;
    }
    else {
        blackBoard.updateWorker(worker->getID(), WorkersAssigment::IDLE);
    }
}

// Send our idle workers to mine minerals so they don't just stand there
void StarterBot::sendIdleWorkersToMinerals()
{
        BWAPI::Unit worker = blackBoard.getIdleWorker();
        if (worker == nullptr) {
            return;
        } else {
            if (worker->isCarryingMinerals()) {
                if (worker->returnCargo()) {
                    blackBoard.updateWorker(worker->getID(), WorkersAssigment::RETURNING_CARGO);
                }
                return;
            }
            BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(worker, BWAPI::Broodwar->getMinerals());
            if (closestMineral) { 
                // std::cout << "CLICKED" << std::endl;
                bool success = worker->gather(closestMineral); 
                if (success) {
                    std::cout << worker->getID() << std::endl;
                    std::cout << closestMineral->getID() << std::endl;
                    blackBoard.updateWorker(worker->getID(), WorkersAssigment::MINNING);
                } else {
                   blackBoard.updateWorker(worker->getID(), WorkersAssigment::IDLE);
                }
                // std::cout << "command finished succesful " << success << std::endl;
                //if (!success) {
                //    blackBoard.updateWorker(worker->getID(), WorkersAssigment::IDLE);
                //}
                // std::cout << "is idle " << worker->isIdle() << std::endl;
                // std::cout << "is constructing " << worker->isConstructing() << std::endl;
                // std::cout << "is moving " << worker->isMoving() << std::endl; // bug ???
                // std::cout << "is gathering " << worker->isGatheringMinerals() << std::endl; // bug ???


            }
            else {
                std::cout << "ALARM NO MINERALS" << std::endl;
            }
    }
}

// Train more workers so we can gather more income
void StarterBot::trainAdditionalWorkers()
{
    const int number_of_workers = blackBoard.getNumberOfWorkers();
    const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();

    const BWAPI::Unit myDepot = Tools::GetDepot(); // TODO CHANGE TO BLACKBOARD

    if (myDepot && !myDepot->isTraining()) { myDepot->train(workerType); }
}

// Build more supply if we are going to run out soon
void StarterBot::buildAdditionalSupply()
{
    std::cout << "CALL BUILD SUPPLY" << std::endl;
    BWAPI::Unit builder = blackBoard.getWorkerForBuilding();
    if (builder == nullptr) {
        std::cout << "no onit found\n";
        return;
    }

    const BWAPI::UnitType supplyType = BWAPI::UnitTypes::Terran_Supply_Depot;

    const bool startedBuilding = Tools::BuildBuilding(supplyType);
    if (startedBuilding) {
            BWAPI::Broodwar->printf("Started Building %s", supplyType.getName().c_str());

            blackBoard.addPlanningBuilding(supplyType, builder->getID());
            blackBoard.updateWorker(builder->getID(), WorkersAssigment::GOING_TO_BUILDING);
    }
}


// Draw some relevent information to the screen to help us debug the bot
void StarterBot::drawDebugInformation()
{
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 10), "Hello, World!\n");
    Tools::DrawUnitCommands();
    Tools::DrawUnitBoundingBoxes();
}

// Called whenever the game ends and tells you if you won or not
void StarterBot::onEnd(bool isWinner)
{
    std::cout << "We " << (isWinner ? "won!" : "lost!") << "\n";
}

// Called whenever a unit is destroyed, with a pointer to the unit
void StarterBot::onUnitDestroy(BWAPI::Unit unit)
{
	
}

// Called whenever a unit is morphed, with a pointer to the unit
// Zerg units morph when they turn into other units
void StarterBot::onUnitMorph(BWAPI::Unit unit)
{
	
}

// Called whenever a text is sent to the game by a user
void StarterBot::onSendText(std::string text) 
{ 
    if (text == "/map")
    {
        m_mapTools.toggleDraw();
    }
}

// Called whenever a unit is created, with a pointer to the destroyed unit
// Units are created in buildings like barracks before they are visible, 
// so this will trigger when you issue the build command for most units
void StarterBot::onUnitCreate(BWAPI::Unit unit)
{ 
    if (unit->getType().isWorker()) {
        blackBoard.addUnit(unit);
    }
    else if (unit->getType().isBuilding()) {
        BWAPI::Position building_pos = unit->getPosition();
        for (int i = 0; i < blackBoard.workers.size(); ++i) {
            if (blackBoard.workers[i] == WorkersAssigment::GOING_TO_BUILDING) {
                std::cout << "We change status for unit who was going to building" << std::endl;
                BWAPI::Unit worker = blackBoard.getWorkerById(blackBoard.worker_ids[i]);
                BWAPI::Position worker_pos = worker->getPosition();
                

                // if (abs(worker_pos.x - building_pos.x) + abs(worker_pos.y - building_pos.y) < 6) { TODO
                blackBoard.updateWorker(worker->getID(), WorkersAssigment::BUILDING);
                std::cout << "we started to build " << unit->getID() << std::endl;
                blackBoard.addBuildingBuilding(unit, worker->getID());
                std::cout << "builder id " << worker->getID() << std::endl;
                //}
            }
        }
    }
}

// Called whenever a unit finished construction, with a pointer to the unit
void StarterBot::onUnitComplete(BWAPI::Unit unit)
{
    if (blackBoard.isWorker(unit)) {
        blackBoard.updateWorker(unit->getID(), WorkersAssigment::IDLE);
    }
    else if (unit->getType().isBuilding()) {
        std::cout << "supply completed " << std::endl;
        int current_id = unit->getID();
        std::cout << "building id " << unit->getID() << std::endl;
        int idx = -1;
        for (int i = 0; i < blackBoard.building.size(); ++i) {
            std::cout << "id of building " << blackBoard.building[i].first->getID()  << std::endl;
            if (blackBoard.building[i].first->getID() == current_id) {
                idx = i;
                std::cout << "we updated worker " << blackBoard.building[i].second << std::endl;
                blackBoard.updateWorker(blackBoard.building[i].second, WorkersAssigment::IDLE);
            }
        }
        if (idx != -1) {
            blackBoard.building.erase(blackBoard.building.begin() + idx);
        }
    }
}

// Called whenever a unit appears, with a pointer to the destroyed unit
// This is usually triggered when units appear from fog of war and become visible
void StarterBot::onUnitShow(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit gets hidden, with a pointer to the destroyed unit
// This is usually triggered when units enter the fog of war and are no longer visible
void StarterBot::onUnitHide(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit switches player control
// This usually happens when a dark archon takes control of a unit
void StarterBot::onUnitRenegade(BWAPI::Unit unit)
{ 
	
}