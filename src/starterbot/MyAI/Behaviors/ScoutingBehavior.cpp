#include "ScoutingBehavior.hpp"

QuotaRequest ScoutEnemyBaseBehaviour::submitQuotaRequest(const BlackBoard& bb) const {
    if (!started && bb.getUnits<WorkerStates>().size() > 10)
        return QuotaRequest{ 100, 1, BWAPI::UnitTypes::Terran_SCV };
    else
        return QuotaRequest{ 100, 0, BWAPI::UnitTypes::Terran_SCV };
}
std::shared_ptr<bt::node> ScoutEnemyBaseBehaviour::createBT(Worker worker, const BlackBoard& bb, Controller& controller) {
    started = true;
    return bt::sequence({
        bt::once([&controller, worker]() {
            BWAPI::Position start_position = worker->unit->getPosition();
            BWAPI::Position target_position = BWAPI::Position();
            target_position.x = (32 * 96) - start_position.x;
            target_position.y = (32 * 128) - start_position.y;
            controller.moveUnit(worker, target_position);
        }),
        bt::wait_until([worker]() {
            return worker->state.inner == WorkerStates::W_IDLE;
        })
    });
}
