#pragma once
#include "Behaviors/ScoutingBehavior.hpp"
#include "Behaviors/ConstructingBehavior.hpp"
#include "Behaviors/GatheringBehavior.hpp"
#include "Behaviors/TrainingBehavior.hpp"
#include "Behaviors/MarineBehavior.hpp"
#include "Behaviors/AttackBehavior.hpp"
#include "Behaviors/UpgradingBehavior.hpp"


// Planner is responsible for the cooperation of all available behaviors.
// It summarizes all the quota requests and resolves conflicts.
struct Planner {
    Planner(): managers() {
        managers.emplace_back(std::make_unique<GatherGasBehavior>());
        managers.emplace_back(std::make_unique<GatherMineralsBehavior>());
        managers.emplace_back(std::make_unique<ConstructingBehavior>());
        managers.emplace_back(std::make_unique<ScoutEnemyBaseBehaviour>());
        managers.emplace_back(std::make_unique<TrainMarinesBehaviour>());
        managers.emplace_back(std::make_unique<TrainWorkersBehaviour>());
        managers.emplace_back(std::make_unique<AttackBehavior>());
        managers.emplace_back(std::make_unique<PushBehavior>());
        managers.emplace_back(std::make_unique<UpgradeMarinesBehaviour>());
        managers.emplace_back(std::make_unique<MoveOnRamp>());
    }


    // Tries to fulfill quota requested from behavior
    template<typename T>
    void assignUnitstoBehavior(std::unique_ptr<Behavior>& b, std::vector<std::shared_ptr<Unit<T>>>& units, Behavior::QuotaRequest quota, const BlackBoard& bb, Controller& controller) {
        int can_provide = std::min(quota.quantity, (int)units.size());
        while (can_provide--) {
            assert(!static_cast<std::shared_ptr<BaseUnit>>(units.back())->isActive);
            b->assign(static_cast<std::shared_ptr<BaseUnit>>(units.back()), bb, controller);
            units.pop_back();
        }
    }

    // Update units according to AI logic
    void update(const BlackBoard& bb, Controller& controller);
    std::vector<std::unique_ptr<Behavior>> managers;
};