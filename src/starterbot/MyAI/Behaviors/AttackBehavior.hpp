#include "BehaviorBase.hpp"

struct AttackBehavior: public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(AttackBehavior)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        auto enemies = bb.getUnits(EnemyStates::E_VISIBLE);
        int totalEnemies = 0;
        for (Enemy enemy : enemies) {
            // if (32 * 40 < enemy->unit->getPosition().getApproxDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()))) {
                totalEnemies++;
            // }
        }
        return QuotaRequest{ 1, 4 * totalEnemies - (int) trees.size(), BWAPI::UnitTypes::Terran_Marine };
    }
    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller) {
        auto enemies = bb.getUnits(EnemyStates::E_VISIBLE);
        for (Enemy enemy : enemies) {
            // if (32 * 40 < enemy->unit->getPosition().getApproxDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()))) {
                nearbyEnemies.push_back(enemy);
            // }
        }
        int idx = rand() % nearbyEnemies.size();
        auto res = bt::once([&controller, idx, marine, this]() {
            controller.attack(marine, nearbyEnemies[idx]);
        });

        return res;
    }


    int current_marines_on_enemy = 0;
    int current_enemy_index = 0;
    std::vector<Enemy> nearbyEnemies;
};

struct PushBehavior: public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(PushBehavior)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        int marines = bb.getUnits(MarineStates::M_IDLE).size();
        if (marines > 5) {
            return QuotaRequest{ 100, marines, BWAPI::UnitTypes::Terran_Marine };
        } else {
            return QuotaRequest{ 0, 0, BWAPI::UnitTypes::Terran_Marine };
        }
    }
    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller) {
        auto res = bt::sequence({
                bt::once([&controller, marine]() {
                    BWAPI::Position start_position = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
                    BWAPI::Position target_position = BWAPI::Position();
                    target_position.x = (32 * 96) - start_position.x;
                    target_position.y = (32 * 128) - start_position.y;
                    controller.moveUnit(marine, target_position);
                }),
                bt::repeat_node_until_success(bt::one_of({
                    bt::if_true([&bb = std::as_const(bb), &controller, marine](){
                        auto enemies = bb.getUnits(EnemyStates::E_VISIBLE);
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
                    bt::if_true([marine]() { return marine->state.inner == MarineStates::M_IDLE; })
                }))
        });
        return res;
    }


    int current_marines_on_enemy = 0;
    int current_enemy_index = 0;
    std::vector<Enemy> nearbyEnemies;
};