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



Behavior::QuotaRequest LonelyScouting::submitQuotaRequest(const BlackBoard& bb) const {
    int marine_count = bb.getUnits<MarineStates>().size();
    int frame_count = BWAPI::Broodwar->getFrameCount();

    if ((marine_count == 1) && (frame_count < 10000)) { // consider it is a early game
        return QuotaRequest{ 2, 1, BWAPI::UnitTypes::Terran_Marine};
    }
    else if (marine_count == 1) {
        return QuotaRequest{ 2, 0, BWAPI::UnitTypes::Terran_Marine };
    }

}

BWAPI::TilePosition LonelyScouting::findTile2Explore(Grid<int>when_seen, BWAPI::TilePosition my_pos, Marine marine) const  {
    BWAPI::TilePosition best_pos(my_pos);
    std::vector<BWAPI::TilePosition> zero_tiles;
    for (int x = 0; x < when_seen.width(); x+=16) {
        for (int y = 0; y < when_seen.height(); y+=16) {

            if (BWAPI::Broodwar->isWalkable(x * 4, y * 4) && marine->unit->hasPath(BWAPI::Position(x * 32, y * 32))) {
            //if (BWAPI::Broodwar->isWalkable(x * 4, y * 4) && BWAPI::Broodwar->getRegionAt(BWAPI::Position(x * 32, y * 32))->isAccessible()) {
                if (when_seen.get(x, y) < when_seen.get(best_pos.x, best_pos.y)) {
                    best_pos = BWAPI::TilePosition(x, y);
                }
                if (when_seen.get(x, y) == 0) {
                    zero_tiles.push_back(BWAPI::TilePosition(x, y));
                }
            }
        }
    }

    int idx = -1;
    int dist = 100000;
    if (zero_tiles.size() > 0) {
        for (int i = 0; i < zero_tiles.size(); ++i) {
            if (my_pos.getApproxDistance(zero_tiles[i]) < dist) {
                dist = my_pos.getApproxDistance(zero_tiles[i]);
                idx = i;
            }
        }
    }

    if (idx == -1) {
        return best_pos;
    }
    else {
        return zero_tiles[idx];
    }
}

std::shared_ptr<bt::node> LonelyScouting::createBT(Marine marine, const BlackBoard& bb, Controller& controller) {        
    return bt::sequence({
            bt::once([&controller, &bb, marine, this]() {
                BWAPI::TilePosition target_position(48, 64);
                marine->framesSinceUpdate = 0;

                controller.moveUnit(marine, BWAPI::Position(target_position));
            }),
            bt::repeat_node_until_success(bt::if_true([marine]() {
                return marine->framesSinceUpdate++ >= 700;
            })),
            bt::repeat_node_until_success(
                bt::sequence({
                    bt::once([&controller, &bb, marine, this]() {
                        BWAPI::TilePosition target_position = findTile2Explore(bb.m_mapTools.m_lastSeen, marine->unit->getTilePosition(), marine);
                        marine->framesSinceUpdate = 0;
                        target_position.x += (rand() % 4) - 2;
                        target_position.x = std::max(0, std::min(target_position.x, 96));
                        target_position.y += (rand() % 4) - 2;
                        target_position.y = std::max(0, std::min(target_position.y, 128));

                        controller.moveUnit(marine, BWAPI::Position(target_position));
                    }),

                    bt::repeat_node_until_success(bt::if_true([marine]() {
                        return marine->framesSinceUpdate++ >= 100;
                    })),
                    bt::fail()
                })
            )
        });
}