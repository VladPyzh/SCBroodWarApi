#pragma once
#include "BehaviorBase.hpp"

struct AttackBehavior: public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(AttackBehavior)

    Behavior::QuotaRequest submitQuotaRequest(const BlackBoard& bb) const;
    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller);
};

struct PushBehavior: public TreeBasedBehavior<MarineStates> {
    DECLARE_STR_TYPE(PushBehavior)

    Behavior::QuotaRequest submitQuotaRequest(const BlackBoard& bb) const ;
    std::shared_ptr<bt::node> createBT(Marine marine, const BlackBoard& bb, Controller& controller) ;
};

struct SupportBehavior : public TreeBasedBehavior<MedicStates> {
    DECLARE_STR_TYPE(PushBehavior)

    QuotaRequest submitQuotaRequest(const BlackBoard& bb) const {
        //std::cerr << "before get" << std::endl;
        int medics = bb.getUnits(MedicStates::Me_IDLE).size();
        
        if (medics > 0) {
            return QuotaRequest{ 50, medics, BWAPI::UnitTypes::Terran_Medic};
        }
        else {
            return QuotaRequest{ 50, 0, BWAPI::UnitTypes::Terran_Medic };
        }
        //std::cerr << "after get" << std::endl;    
        
    }

    std::shared_ptr<bt::node> createBT(Medic medic, const BlackBoard& bb, Controller& controller) {
        
        auto medics = bb.m_medics;
        int i = 0;
        for (; i < medics.size(); ++i) {
            if (medics[i] == medic) {
                break;
            }
        }
        auto marines = bb.m_marines;
        auto my_marine = marines[std::min(4 + i, (int)marines.size() - 1)];

        return  bt::one_of({
                bt::if_true([medic]() {
                    if (medic->framesSinceUpdate == 0) {
                        return false;
                    }
                    else {
                        if (medic->framesSinceUpdate >= 15) {
                            medic->framesSinceUpdate = 0;
                            return true;
                        }
                        else {
                            medic->framesSinceUpdate++;
                            return true;
                        }

                    }
                }),
                bt::if_true([&controller, medic, &bb = std::as_const(bb), this]() {
                    std::cerr << "in Seq" << std::endl;
                    if (medic->framesSinceUpdate >= 1) {
                        return true;
                    }
                    int radius = 3 * 32;
                    BWAPI::Unitset unitsInRadius = medic->unit->getUnitsInRadius(radius);

                    for (auto unit : unitsInRadius) {
                        if (unit->isUnderAttack() && unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()) == false) {
                            medic->unit->rightClick(unit);
                            medic->framesSinceUpdate = 1;
                            return true;
                        }
                    }
                    return false;
                }),
                bt::repeat_until_success([&controller, medic, my_marine, &bb = std::as_const(bb), this]() {
                    return controller.moveUnit(medic, my_marine->unit->getPosition());
                }),
                bt::fail()
            });
    }

};