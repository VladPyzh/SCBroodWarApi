#include "Behaviors/ScoutingBehavior.hpp"
#include "Behaviors/ConstructingBehavior.hpp"
#include "Behaviors/GatheringBehavior.hpp"
#include "Behaviors/TrainingBehavior.hpp"
#include "Behaviors/MarineBehavior.hpp"

struct Planner {
    Planner(): managers() {
        managers.emplace_back(std::make_unique<GatherGasBehavior>());
        managers.emplace_back(std::make_unique<GatherMineralsBehavior>());
        managers.emplace_back(std::make_unique<ConstructingBehavior>());
        // managers.emplace_back(std::make_unique<ScoutEnemyBaseBehaviour>());
        managers.emplace_back(std::make_unique<TrainMarinesBehaviour>());
        managers.emplace_back(std::make_unique<TrainWorkersBehaviour>());
        managers.emplace_back(std::make_unique<MoveOnRamp>()); // stupidest thing
    }

    template<typename T>
    void assignUnitstoBehavior(std::unique_ptr<Behavior>& b, std::vector<std::shared_ptr<Unit<T>>>& units, Behavior::QuotaRequest quota, const BlackBoard& bb, Controller& controller) {
        int can_provide = std::min(quota.quantity, (int)units.size());
        while (can_provide--) {
            assert(!static_cast<std::shared_ptr<BaseUnit>>(units.back())->isActive);
            b->assign(static_cast<std::shared_ptr<BaseUnit>>(units.back()), bb, controller);
            units.pop_back();
        }
    }

    void update(const BlackBoard& bb, Controller& controller) {
        std::vector<Behavior::QuotaRequest> requests;
        for (int i = 0; i < managers.size(); i++) {
            auto& manager = managers[i];
            auto q = manager->submitQuotaRequest(bb);
            q.idx = i;
            requests.push_back(q);
        }
        std::sort(requests.rbegin(), requests.rend(), [](Behavior::QuotaRequest a, Behavior::QuotaRequest b) {
            return a.priority < b.priority;
        });
        std::vector<Worker> workers = bb.getUnits(W_IDLE);
        std::vector<Depot> depots = bb.getUnits(D_IDLE);
        std::vector<Barrack> barracks = bb.getUnits(B_IDLE);
        std::vector<Marine> marines = bb.getUnits(M_IDLE);

        for (int i = 0; i < requests.size(); i++) {
            if (requests[i].type == BWAPI::UnitTypes::Terran_SCV) {
                assignUnitstoBehavior(managers[requests[i].idx], workers, requests[i], bb, controller);
            }
            if (requests[i].type == BWAPI::UnitTypes::Terran_Supply_Depot) {
                assignUnitstoBehavior(managers[requests[i].idx], depots, requests[i], bb, controller);
            }
            if (requests[i].type == BWAPI::UnitTypes::Terran_Barracks) {
                assignUnitstoBehavior(managers[requests[i].idx], barracks, requests[i], bb, controller);
            }
            if (requests[i].type == BWAPI::UnitTypes::Terran_Marine) {
                assignUnitstoBehavior(managers[requests[i].idx], marines, requests[i], bb, controller);
            }
        }
        
        for (const auto& manager : managers) {
            manager->update(bb, controller);
        }
       
    }

    std::vector<std::unique_ptr<Behavior>> managers;
};