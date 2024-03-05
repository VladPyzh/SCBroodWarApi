#include "BehaviorBase.hpp"

struct GatherMineralsBehavior: public TreeBasedBehavior<WorkerStates> {
    DECLARE_STR_TYPE(GatherMineralsBehavior)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        return QuotaRequest{ 1, 100, BWAPI::UnitTypes::Terran_SCV };
    }
    std::shared_ptr<bt::node> createBT(Worker worker, const BlackBoard& bb, Controller& controller) {
        return 
            bt::one_of({
                bt::sequence({
                    bt::if_true([worker]() {
                        return worker->state.inner == WorkerStates::W_IS_TO_RETURN_CARGO;
                    }),
                    bt::repeat_until_success([worker, &controller]() {
                        return controller.returnCargo(worker);
                    }),
                     bt::wait_until([worker]() {
                        return worker->state.inner == WorkerStates::W_IDLE;
                    })
                }),
                bt::sequence({
                    bt::if_true([worker, &controller]() {
                        return controller.harvestMinerals(worker);
                    }),
                    bt::wait_until([worker]() {
                        return worker->state.inner == WorkerStates::W_IS_TO_RETURN_CARGO;
                    }),
                    bt::if_true([worker]() {
                        return worker->state.inner == WorkerStates::W_IS_TO_RETURN_CARGO;
                    }),
                    bt::repeat_until_success([worker, &controller]() {
                        return controller.returnCargo(worker);
                    }),
                     bt::wait_until([worker]() {
                        return worker->state.inner == WorkerStates::W_IDLE;
                    })
                }),
                bt::once([&controller, worker]() {
                    controller.moveUnit(worker, BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
                })
            });
    }
};
