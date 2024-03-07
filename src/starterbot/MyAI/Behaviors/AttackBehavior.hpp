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