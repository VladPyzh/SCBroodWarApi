#include "ScoutingBehavior.hpp"

Behavior::QuotaRequest ScoutEnemyBaseBehaviour::submitQuotaRequest(const BlackBoard& bb) const {
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



Behavior::QuotaRequest submitQuotaRequest(const BlackBoard& bb) const ; {
    int marine_count = bb.getUnits<MarineStates>().size();
    int frame_count = BWAPI::Broodwar->getFrameCount();

    if ((marine_count == 1) && (frame_count < 10000)) { // consider it is a early game
        return QuotaRequest{ 1, 1, BWAPI::UnitTypes::Terran_Marine};
    }
    else if (marine_count == 1) {
        return QuotaRequest{ 1, 0, BWAPI::UnitTypes::Terran_Marine };
    }

}

BWAPI::TilePosition findTile2Explore(Grid<int>when_seen, BWAPI::TilePosition my_pos) const ; {
    BWAPI::TilePosition best_pos(my_pos);
    for (int x = 0; x < when_seen.width(); ++x) {
        for (int y = 0; y < when_seen.height(); ++y) {
            if (BWAPI::Broodwar->isWalkable(x * 32, y * 32)) {
                if (when_seen.get(best_pos.x, best_pos.y) == 0) {
                    return best_pos;
                }
                if (when_seen.get(x, y) < when_seen.get(best_pos.x, best_pos.y)) {
                    best_pos = BWAPI::TilePosition(x, y);
                }
            }
        }
    }
    return best_pos;
}

std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller) ; {        
    return bt::repeat_node_until_success( bt::sequence({
        bt::once([&controller, &bb, marine, this]() {
            BWAPI::TilePosition target_position = findTile2Explore(bb.m_mapTools.m_lastSeen, marine->unit->getTilePosition());
            marine->framesSinceUpdate = 0;
            controller.moveUnit(marine, BWAPI::Position(target_position));
        }),
        bt::repeat_node_until_success( bt::once([marine]() {

            return 20 == marine->framesSinceUpdate++;
        })
        )
    })
    );
}