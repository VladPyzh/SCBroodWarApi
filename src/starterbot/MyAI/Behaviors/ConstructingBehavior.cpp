
#include "ConstructingBehavior.hpp"

const bool GEYSER_DEBUG = false;

BWAPI::TilePosition findClosestVespeneGeyser(BWAPI::Unit worker) {
    BWAPI::Unit closestGeyser = nullptr;
    double minDistance = std::numeric_limits<double>::max();

    for (auto& unit : BWAPI::Broodwar->getNeutralUnits()) {
        if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
            double distance = worker->getPosition().getDistance(unit->getPosition());
            if (distance < minDistance) {
                closestGeyser = unit;
                minDistance = distance;
            }
        }
    }

    DEBUG_LOG(GEYSER_DEBUG, closestGeyser->getTilePosition().x << " " << closestGeyser->getTilePosition().y << std::endl)


    return closestGeyser->getTilePosition();
}


bool ConstructingBehavior::canConstruct(const BlackBoard& bb, BWAPI::UnitType type) const {
    int have_minerals = bb.minerals();
    int have_gas = bb.gas();
    int cost_mineral = type.mineralPrice();
    int cost_gas = type.gasPrice();

    return have_minerals >= cost_mineral && have_gas >= cost_gas;
}

bool ConstructingBehavior::shouldConstruct(const BlackBoard& bb, BWAPI::UnitType type) const {
    int unitSlots = bb.freeUnitSlots();
    // Todo: approximate that in X seconds we will outrun of unit slots
    // based on current unit production speed
    return canConstruct(bb, type) && unitSlots < 10;
}

Behavior::QuotaRequest ConstructingBehavior::submitQuotaRequest(const BlackBoard& bb) const {
    for (auto unitType : bb.pending_units) {
        if (unitType.isBuilding()) {
            return QuotaRequest{ 2, 0, BWAPI::UnitTypes::Terran_SCV };
        }
    }
    if (cur_build_index < build_order.size() && canConstruct(bb, build_order[cur_build_index])) {
        // dont try to start with 2 builders, use one wisely
        if (cur_build_index == 0) {
            return QuotaRequest{ 5, std::max(0, 1 - (int)trees.size()), BWAPI::UnitTypes::Terran_SCV };
        }
        else {
            return QuotaRequest{ 5, std::max(0, 2 - (int)trees.size()), BWAPI::UnitTypes::Terran_SCV };
        }
    } else if (cur_build_index >= build_order.size() && shouldConstruct(bb, BWAPI::UnitTypes::Terran_Supply_Depot)) {
        return QuotaRequest{ 2, std::max(0, 5 - (int)trees.size()), BWAPI::UnitTypes::Terran_SCV };
    } else {
        return QuotaRequest{ 2, 0, BWAPI::UnitTypes::Terran_SCV };
    }
}

std::shared_ptr<bt::node> ConstructingBehavior::createBT(Worker worker, const BlackBoard& bb, Controller& controller) {
    if (cur_build_index < build_order.size()) {
        int tree_for_building_idx = cur_build_index;
        BWAPI::Position position = (BWAPI::Broodwar->self()->getStartLocation().x < 48) ? BWAPI::Position(47 * 32, 8 * 32) : BWAPI::Position(42 * 32, 117 * 32);
        return bt::sequence({
            bt::one_of({
                bt::if_true([&bb = std::as_const(bb), position]() {
                    return bb.m_mapTools.isExplored(position);
                }),
                bt::sequence({
                    bt::once([&controller, worker, position](){
                        controller.moveUnit(worker, position);
                    }),
                    bt::wait_until([worker]() {
                        return worker->state.inner == W_IDLE;
                    })
                }),
            }),
            bt::repeat_node_until_success(bt::one_of({
                bt::if_true([this, tree_for_building_idx]() {
                    // std::cerr << cur_build_index << ' ' << tree_for_building_idx << std::endl;
                    return cur_build_index > tree_for_building_idx;
                }),
                bt::sequence({
                    bt::if_true([this, tree_for_building_idx]() {
                        // std::cerr << "chech cur build index " << cur_build_index << ' ' << tree_for_building_idx  << '\n';
                        return cur_build_index == tree_for_building_idx;
                    }),
                    bt::if_true([worker, &bb = std::as_const(bb), this, tree_for_building_idx]() {
                        // auto pos = BWAPI::Broodwar->getBuildLocation(build_order[tree_for_building_idx], positions[tree_for_building_idx], 64, false);

                        if (build_order[tree_for_building_idx] == BWAPI::UnitTypes::Terran_Refinery) { // Corner case for refinery
                            BWAPI::TilePosition position = findClosestVespeneGeyser(worker->unit);
                            bool res = canConstruct(bb, build_order[tree_for_building_idx]) &&
                                worker->unit->canBuild(build_order[tree_for_building_idx], position);
                            BWAPI_LOG_IF_ERROR()
                            return res;
                        }
                        else {
                            bool res = canConstruct(bb, build_order[tree_for_building_idx]) &&
                                worker->unit->canBuild(build_order[tree_for_building_idx], positions[tree_for_building_idx]);
                            BWAPI_LOG_IF_ERROR()
                            return res;
                        }
                    }),
                    bt::if_true([&controller, worker, &bb = std::as_const(bb), this, tree_for_building_idx]() {
                        if (build_order[tree_for_building_idx] == BWAPI::UnitTypes::Terran_Refinery) { // Corner case for refinery
                            BWAPI::TilePosition position = findClosestVespeneGeyser(worker->unit);
                            return controller.build(worker, build_order[tree_for_building_idx], position, bb);
                        }
                        else {
                            return controller.build(worker, build_order[tree_for_building_idx], positions[tree_for_building_idx], bb);
                        }
                    }),
                    bt::once([this, tree_for_building_idx]() {
                        cur_build_index++;
                        BWAPI::Broodwar->printf("Start building: %s", build_order[tree_for_building_idx].getName().c_str());
                    }),
                    bt::wait_until([worker]() {
                        // std::cerr << "wait idle " << worker->state.inner << std::endl;
                        // std::cerr << worker->unit->isVisible() << std::endl;
                        return worker->state.inner == W_BUILDING;
                    }),
                    bt::wait_until([worker]() {
                        // std::cerr << "wait idle " << worker->state.inner << std::endl;
                        // std::cerr << worker->unit->isVisible() << std::endl;
                        // if (worker->state.inner != WorkerStates::W_BUILDING) {
                        //     std::cerr << worker->state.inner << std::endl;
                        //      throw std::runtime_error("bad state");
                        // }
                        return worker->state.inner == W_IDLE;
                    })
                })
            }))
        });
    } else {
        return bt::try_node(    
            bt::sequence({
                bt::if_true([&bb = std::as_const(bb), this]() {
                    return shouldConstruct(bb, BWAPI::UnitTypes::Terran_Supply_Depot);
                }),
                bt::repeat_until_success([worker, &controller, &bb]() {
                    BWAPI::TilePosition desiredPos = BWAPI::Broodwar->self()->getStartLocation();
                    int maxBuildRange = 64;
                    return controller.build(worker, BWAPI::UnitTypes::Terran_Supply_Depot, BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Terran_Supply_Depot, desiredPos, maxBuildRange, false), bb);
                }),
                bt::wait_until([worker]() {
                    return worker->state.inner == WorkerStates::W_IDLE;
                })
            })
        );
    }
}
