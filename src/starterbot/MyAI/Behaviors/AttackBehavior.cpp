#include "AttackBehavior.hpp"

Behavior::QuotaRequest AttackBehavior::submitQuotaRequest(const BlackBoard& bb) const {
    auto enemies = bb.getUnits(EnemyStates::E_VISIBLE);
    int totalEnemies = 0;
    for (Enemy enemy : enemies) {
        if (12 > enemy->unit->getPosition().getApproxDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()))) {
            totalEnemies++;
        }
    }
    return QuotaRequest{ 100, 10 * totalEnemies - (int) trees.size(), BWAPI::UnitTypes::Terran_Marine };
}
std::shared_ptr<bt::node> AttackBehavior::createBT(Marine marine, const BlackBoard& bb, Controller& controller) {
    auto enemies = bb.getUnits(EnemyStates::E_VISIBLE);
    std::vector<Enemy> nearbyEnemies;
    std::vector<int> distances;

    for (Enemy enemy : enemies) {
        int dist = enemy->unit->getPosition().getApproxDistance(BWAPI::Position(marine->unit->getPosition()));
        if (10 > dist) {
            nearbyEnemies.push_back(enemy);
            distances.push_back(dist);
        }
    }
    int min = 1000;
    int argmin = 0;
    for (int i = 0; i < distances.size(); ++i) {
        if (distances[i] < min) {
            argmin = i;
            min = distances[i];
        }
    }        

    int idx = argmin;
    auto res = bt::sequential({
        bt::once([&controller, idx, marine, nearbyEnemies, this]() {
            controller.attack(marine, nearbyEnemies[idx]);
        }),
        bt::wait_until([marine](){
            return marine->state.inner == M_IDLE;
        })
    });

    return res;
}


Behavior::QuotaRequest PushBehavior::submitQuotaRequest(const BlackBoard& bb) const {
    int marines = bb.getUnits(MarineStates::M_IDLE).size();

    if (marines > 20) {
        return QuotaRequest{ 50, marines, BWAPI::UnitTypes::Terran_Marine };
    } else {
        return QuotaRequest{ 0, 0, BWAPI::UnitTypes::Terran_Marine };
    }
}
std::shared_ptr<bt::node> PushBehavior::createBT(Marine marine, const BlackBoard& bb, Controller& controller) {
    auto res = bt::repeat_node_until_success(bt::one_of({
            bt::sequence({
                bt::if_true([&bb = std::as_const(bb), &controller, marine](){
                    auto enemies = bb.getUnits(EnemyStates::E_VISIBLE);
                    std::vector<Enemy> nearbyEnemies;
                    for (Enemy enemy : enemies) {
                        if (12 * 32 > enemy->unit->getPosition().getApproxDistance(BWAPI::Position(marine->unit->getPosition()))) {
                            nearbyEnemies.push_back(enemy);
                        }
                    }
                    enemies = nearbyEnemies;
                    int idx = -1;
                    for (int i = 0; i < enemies.size(); i++) {
                        auto cur_dist = enemies[i]->unit->getPosition().getApproxDistance(marine->unit->getPosition());
                        if (idx == -1 || cur_dist < enemies[idx]->unit->getPosition().getApproxDistance(marine->unit->getPosition())) {
                            idx = i;
                        }
                    }
                    if (idx != -1) {
                        controller.attack(marine, enemies[idx]);
                        return true;
                    } else {
                        return false;
                    }
                }),
                bt::repeat_until_success([marine]() { return marine->state.inner == MarineStates::M_IDLE; }),
                bt::fail()
            }),
            bt::sequence({
                bt::if_true([marine, this](){
                    BWAPI::Position start_position = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
                    BWAPI::Position target_position = BWAPI::Position();
                    target_position.x = (32 * 96) - start_position.x;
                    target_position.y = (32 * 128) - start_position.y;
                    
                    auto marinesInSquad = getUnitGroup(marine);
                    auto myDist = marine->unit->getPosition().getApproxDistance(target_position);
                    for (auto unit : marinesInSquad) {
                        if (unit->unit->getPosition().getApproxDistance(target_position) < myDist) {
                            return false;
                        }
                    }
                    // DEBUG_LOG(true, "unit " << marine->unit->getID() << " is leader, stop at " << marine->unit->getPosition() << '\n');
                    return true;
                }),

                bt::if_true([marine, this](){
                    auto marinesInSquad = getUnitGroup(marine);
                    int cntBad = 0;
                    for (auto unit : marinesInSquad) {
                        if (unit->unit->getPosition().getApproxDistance(marine->unit->getPosition()) > 32 * 2) {
                            cntBad++;
                        }
                    }
                    // DEBUG_LOG(true, "squad has " << cntBad << " bad units" << '\n');
                    return marinesInSquad.size() >= 20 && cntBad * 10 >= marinesInSquad.size();
                }),
                bt::once([&controller, marine, this]() {
                    controller.stop(marine);
                }),
                bt::repeat_until_success([marine, this]() {
                    BWAPI::Position start_position = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
                    BWAPI::Position target_position = BWAPI::Position();
                    target_position.x = (32 * 96) - start_position.x;
                    target_position.y = (32 * 128) - start_position.y;
                    
                    auto marinesInSquad = getUnitGroup(marine);
                    auto myDist = marine->unit->getPosition().getApproxDistance(target_position);
                    if (marinesInSquad.size() < 5) {
                        return true;
                    }
                    for (auto unit : marinesInSquad) {
                        if (unit->unit->getPosition().getApproxDistance(target_position) < myDist) {
                            // DEBUG_LOG(true, "unit " << marine->unit->getID() << " no longer a leader" << '\n');
                            return true;
                        }
                    }
                    return false;
                }),
                bt::fail()
            }),
            bt::sequence({
                bt::one_of({
                    bt::if_true([marine](){return marine->state.inner == MarineStates::M_MOVING;}),
                    bt::once([&controller, marine]() {
                        BWAPI::Position start_position = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
                        BWAPI::Position target_position = BWAPI::Position();
                        target_position.x = (32 * 96) - start_position.x;
                        target_position.y = (32 * 128) - start_position.y;
                        controller.moveUnit(marine, target_position);
                    })
                }),
                bt::if_true([marine]() { return marine->state.inner == MarineStates::M_IDLE; })
            })
    }));
    return res;
}

