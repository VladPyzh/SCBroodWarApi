#pragma once

#include "BB.hpp"
#include "Controller.hpp"
#include "Units.hpp"
#include "MapTools.h"
#include "bt.hpp"
#include "Log.hpp"

#include <vector>
#include <memory>
#include <cassert>


constexpr bool ASSIGN_BT_DEBUG = true;
constexpr bool UPDATE_BT_DEBUG = false;

struct Behavior {
    
    struct QuotaRequest {
        int priority;
        int quantity;
        BWAPI::UnitType type;
        int idx = -1;
    };
    virtual QuotaRequest submitQuotaRequest(const BlackBoard& bb) const = 0;
    virtual void update(const BlackBoard& bb, Controller& controller) = 0;
    virtual void assign(std::shared_ptr<BaseUnit> unit, const BlackBoard& bb, Controller& controller) = 0;
    virtual ~Behavior() {}
};



template
<typename T>
struct TreeBasedBehavior: public Behavior {
    virtual std::shared_ptr<bt::node> createBT(std::shared_ptr<Unit<T>> unit, const BlackBoard& bb, Controller& controller) = 0;
    virtual std::string type() = 0;
    void assign(std::shared_ptr<BaseUnit> unit, const BlackBoard& bb, Controller& controller) {
        std::shared_ptr<Unit<T>> unit_ptr = std::dynamic_pointer_cast<Unit<T>>(unit);
        unit_ptr->isActive = true;
        DEBUG_LOG(ASSIGN_BT_DEBUG, unit_ptr->unit->getID() << ' ' << type() << ' ' << '\t')
        trees.push_back(createBT(unit_ptr, bb, controller));
        units.push_back(unit_ptr);
        DEBUG_CALL(ASSIGN_BT_DEBUG, trees.back()->print())
        DEBUG_LOG(ASSIGN_BT_DEBUG, std::endl)
    }
    void update(const BlackBoard& bb, Controller& controller) {
        std::vector<int> to_delete;
        DEBUG_LOG(UPDATE_BT_DEBUG, type() << ' ');
        for (int i = 0; i < trees.size(); i++) {
            auto tree = trees[i];
            DEBUG_LOG(UPDATE_BT_DEBUG, units[i]->unit->getID() << ' ' << units[i]->state.inner << ", ");
            auto status = tree->step();

            if (status == bt::state::failure) {
                std::cerr << units[i]->unit->getID() << ' ' << units[i]->state.inner << '\n';
                tree->print(true);
                std::cerr << '\n';
                tree->printStack(true);
                std::cerr << '\n';
                throw std::runtime_error("behavior not allowed to fail");
            }
            if (status != bt::state::running) {
                units[i]->isActive = false;
                trees.erase(trees.begin() + i);
                units.erase(units.begin() + i);
                i--;
                continue;
            }
        }
        DEBUG_LOG(UPDATE_BT_DEBUG, '\n');
    }
    std::vector<std::shared_ptr<bt::node>> trees;
    std::vector<std::shared_ptr<Unit<T>>> units;
};