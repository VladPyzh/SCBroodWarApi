#include "Planners.hpp"


void Planner::update(const BlackBoard& bb, Controller& controller) {
    // gather quota requests from all behaviors
    std::vector<Behavior::QuotaRequest> requests;
    for (int i = 0; i < managers.size(); i++) {
        auto& manager = managers[i];
        auto q = manager->submitQuotaRequest(bb);
        q.idx = i;
        requests.push_back(q);
    }
    // we sort quota requests based on their priority
    std::sort(requests.rbegin(), requests.rend(), [](Behavior::QuotaRequest a, Behavior::QuotaRequest b) {
        return a.priority < b.priority;
    });

    // distribute available units between behaviors
    std::vector<Worker> workers = bb.getUnits(W_IDLE);
    std::vector<Depot> depots = bb.getUnits(D_IDLE);
    std::vector<Barrack> barracks = bb.getUnits(B_IDLE);
    std::vector<Marine> marines = bb.getUnits(M_IDLE);
    std::vector<Academy> academies = bb.getUnits(A_IDLE);

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
        if (requests[i].type == BWAPI::UnitTypes::Terran_Academy) {
            assignUnitstoBehavior(managers[requests[i].idx], academies, requests[i], bb, controller);
        }
    }
    
    // update units
    for (const auto& manager : managers) {
        manager->update(bb, controller);
    }
}
