#include "BehaviorBase.hpp"

struct DefendBehavior: public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(DefendBehavior)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        return QuotaRequest{ 0, 0, BWAPI::UnitTypes::Terran_Marine };
    }
    bool needToReact(Marine marine) {
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
        return nearbyEnemies.size() > 0;
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
        auto res = bt::sequence({
            bt::once([&controller, idx, marine, nearbyEnemies, this]() {
                controller.stop(marine);
            }),
            bt::wait_until([marine]() {
                return marine->framesSinceUpdate > 2;
            }),
            bt::once([&controller, idx, marine, nearbyEnemies, this]() {
                controller.attack(marine, nearbyEnemies[idx]);
            })
        }) 

        return res;
    }
};