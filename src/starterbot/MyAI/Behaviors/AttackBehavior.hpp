#include "BehaviorBase.hpp"

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
        return QuotaRequest{ 1, 4 * totalEnemies - (int) trees.size(), BWAPI::UnitTypes::Terran_Marine };
    }
    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller) {
        auto enemies = bb.getUnits(EnemyStates::E_VISIBLE);
        std::vector<Enemy> nearbyEnemies;
        for (Enemy enemy : enemies) {
            if (12 > enemy->unit->getPosition().getApproxDistance(BWAPI::Position(marine->unit->getPosition()))) {
                nearbyEnemies.push_back(enemy);
            }
        }
        int idx = rand() % nearbyEnemies.size();
        auto res = bt::once([&controller, idx, marine, &nearbyEnemies, this]() {
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
            return QuotaRequest{ 100, marines, BWAPI::UnitTypes::Terran_Marine };
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
                        auto marinesInSquad = getUnitGroup(marine);
                        for (auto unit : marinesInSquad) {
                            if (unit->unit->getPosition().getApproxDistance(marine->unit->getPosition()) > 32 * 15) {
                                return true;
                            }
                        }
                        return false;
                    }),
                    bt::once([&controller, marine, this]() {
                        auto marinesInSquad = getUnitGroup(marine);
                        // auto avg = BWAPI::Position(0, 0);
                        // for (auto unit : marinesInSquad) {
                        //     avg += unit->unit->getPosition();
                        // }
                        controller.moveUnit(marine, marinesInSquad[0]->unit->getPosition());
                    }),
                    bt::repeat_until_success([marine, &controller, this](){
                        int cntMarines = 0;
                        auto marinesInSquad = getUnitGroup(marine);

                        for (auto unit : marinesInSquad) {
                            if (unit->unit->getPosition().getApproxDistance(marine->unit->getPosition()) > 32 * 10) {
                                return false;
                            }
                        }
                        controller.stop(marine);
                        return true;
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

    std::vector<Enemy> nearbyEnemies;
};
