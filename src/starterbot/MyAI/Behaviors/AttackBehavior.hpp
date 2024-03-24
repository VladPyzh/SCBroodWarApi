#include "BehaviorBase.hpp"


struct MoveOnRamp : public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(MoveOnRamp)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        auto unitsInRadius = BWAPI::Broodwar->getUnitsInRadius(ramp_location, 4 * 32);
        int cntMarines = 0;
        for (auto unit : unitsInRadius) {
            if (unit->getType() == BWAPI::UnitTypes::Terran_Marine) {
                cntMarines++;
            }
        }
        if (cntMarines < 7) {
            return QuotaRequest{ 1, 7 - cntMarines, BWAPI::UnitTypes::Terran_Marine };
        }
        else {
            return QuotaRequest{ 1, 0, BWAPI::UnitTypes::Terran_Marine };
        }

    }

    BWAPI::Position ramp_location = (BWAPI::Broodwar->self()->getStartLocation().x < 48) ? BWAPI::Position(54 * 32, 11 * 32) : BWAPI::Position(42 * 32, 117 * 32);

    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller) {
        auto unitsInRadius = BWAPI::Broodwar->getUnitsInRadius(ramp_location, 20 * 32);
        int cntMarines = 0;
        for (auto unit : unitsInRadius) {
            if (unit->getType() == marine->unit->getType()) {
                cntMarines++;
            }
        }
        int distanceNum = cntMarines / 4 + 1;
        return
            bt::one_of({
                bt::if_true([marine, this, distanceNum]() {
                    return marine->unit->getPosition().getApproxDistance(ramp_location) < distanceNum * 32;
                }),
                bt::sequence({
                    bt::repeat_until_success([&controller, marine, &bb = std::as_const(bb), this]() {
                        return controller.moveUnit(marine, ramp_location);
                    }),
                    bt::repeat_node_until_success(bt::sequence({
                        bt::one_of({
                            bt::if_true([marine, this, distanceNum]() {
                                return marine->unit->getPosition().getApproxDistance(ramp_location) < distanceNum * 32;
                            }),
                            bt::if_true([marine]() {
                                return marine->framesSinceUpdate > 200;
                            })
                        }),
                        bt::once([marine, &controller]() {
                            controller.stop(marine);
                        }),
                    }))
                }),
                });
    }
};


struct AttackBehavior: public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(AttackBehavior)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        auto enemies = bb.getUnits(EnemyStates::E_VISIBLE);
        int totalEnemies = 0;
        for (Enemy enemy : enemies) {
            if (12 > enemy->unit->getPosition().getApproxDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()))) {
                totalEnemies++;
            }
        }
        return QuotaRequest{ 100, 4 * totalEnemies - (int) trees.size(), BWAPI::UnitTypes::Terran_Marine };
    }
    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller) {
        auto enemies = bb.getUnits(EnemyStates::E_VISIBLE);
        std::vector<Enemy> nearbyEnemies;
        std::vector<int> distances;

        for (Enemy enemy : enemies) {
            int dist = enemy->unit->getPosition().getApproxDistance(BWAPI::Position(marine->unit->getPosition()));
            if (6 > dist) {
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
        auto res = bt::once([&controller, idx, marine, nearbyEnemies, this]() {
            controller.attack(marine, nearbyEnemies[idx]);
        });

        return res;
    }
};

struct PushBehavior: public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(PushBehavior)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        int marines = bb.getUnits(MarineStates::M_IDLE).size();

        if (marines > 20) {
            return QuotaRequest{ 50, marines, BWAPI::UnitTypes::Terran_Marine };
        } else {
            return QuotaRequest{ 0, 0, BWAPI::UnitTypes::Terran_Marine };
        }
    }
    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller) {
        auto res = bt::repeat_node_until_success(bt::one_of({
                bt::sequence({
                    bt::if_true([&bb = std::as_const(bb), &controller, marine](){
                        auto enemies = bb.getUnits(EnemyStates::E_VISIBLE);
                        std::vector<Enemy> nearbyEnemies;
                        for (Enemy enemy : enemies) {
                            if (12 * 32 > enemy->unit->getPosition().getApproxDistance(BWAPI::Position(marine->unit->getPosition()))) {
                                if (enemy->unit->getType() == BWAPI::UnitTypes::Zerg_Egg || enemy->unit->getType() == BWAPI::UnitTypes::Zerg_Overlord) {
                                    continue;
                                }
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
                        return (marinesInSquad.size() >= 20) && (cntBad * 5 >= marinesInSquad.size());
                    }),
                    /*
                    bt::if_true([marine, this]() {
                        auto marinesInSquad = getUnitGroup(marine);
                        int cntBad = 0;
                        for (auto unit1 : marinesInSquad) {
                            for (auto unit2 : marinesInSquad) {
                                if (unit1->unit->getPosition().getApproxDistance(unit2->unit->getPosition()) > 32 * 4) {
                                    cntBad++;
                                }
                            }
                        }
                        // DEBUG_LOG(true, "squad has " << cntBad << " bad units" << '\n');
                        return cntBad * 10 >= marinesInSquad.size();
                    }),*/
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
                        bt::if_true([marine, &bb = std::as_const(bb)](){return marine->state.inner == MarineStates::M_MOVING && bb.m_enemies.size() > 0;}),
                        bt::once([&controller, marine, &bb = std::as_const(bb)]() {
                            BWAPI::Position start_position = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
                            BWAPI::Position target_position = BWAPI::Position();

                            /*
                            auto enemies = bb.getUnits(EnemyStates::E_UNKNOWN);
                            auto known_enemies = bb.getUnits(EnemyStates::E_VISIBLE);
                            enemies.insert(enemies.end(), known_enemies.begin(), known_enemies.end());
                            int idx = -1;
                            for (int i = 0; i < enemies.size(); i++) {
                                auto cur_dist = enemies[i]->unit->getPosition().getApproxDistance(marine->unit->getPosition());
                                if (idx == -1 || cur_dist < enemies[idx]->unit->getPosition().getApproxDistance(marine->unit->getPosition())) {
                                    idx = i;
                                }
                            }

                            target_position = enemies[idx]->unit->getPosition();
                            */

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

    std::vector<Enemy> nearbyEnemies;
};

struct SupportBehavior : public TreeBasedBehavior<MedicStates> {
    DECLARE_STR_TYPE(PushBehavior)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        //std::cerr << "before get" << std::endl;
        int medics = bb.getUnits(MedicStates::Me_IDLE).size();
        
        if (medics > 0) {
            return QuotaRequest{ 50, medics, BWAPI::UnitTypes::Terran_Medic};
        }
        else {
            return QuotaRequest{ 50, 0, BWAPI::UnitTypes::Terran_Medic };
        }
        //std::cerr << "after get" << std::endl;    
        
    }

    std::shared_ptr<bt::node> createBT(Medic medic, const BlackBoard& bb, Controller& controller) {
        
        auto medics = bb.m_medics;
        int i = 0;
        for (; i < medics.size(); ++i) {
            if (medics[i] == medic) {
                break;
            }
        }
        auto marines = bb.m_marines;
        auto my_marine = marines[std::min(4 + i, (int)marines.size() - 1)];

        return  bt::one_of({
                bt::if_true([medic]() {
                    if (medic->framesSinceUpdate == 0) {
                        return false;
                    }
                    else {
                        if (medic->framesSinceUpdate >= 15) {
                            medic->framesSinceUpdate = 0;
                            return true;
                        }
                        else {
                            medic->framesSinceUpdate++;
                            return true;
                        }

                    }
                }),
                bt::if_true([&controller, medic, &bb = std::as_const(bb), this]() {
                    std::cerr << "in Seq" << std::endl;
                    if (medic->framesSinceUpdate >= 1) {
                        return true;
                    }
                    int radius = 3 * 32;
                    BWAPI::Unitset unitsInRadius = medic->unit->getUnitsInRadius(radius);

                    for (auto unit : unitsInRadius) {
                        if (unit->isUnderAttack() && unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()) == false) {
                            medic->unit->rightClick(unit);
                            medic->framesSinceUpdate = 1;
                            return true;
                        }
                    }
                    return false;
                }),
                bt::repeat_until_success([&controller, medic, my_marine, &bb = std::as_const(bb), this]() {
                    return controller.moveUnit(medic, my_marine->unit->getPosition());
                }),
                bt::fail()
            });
    }

};