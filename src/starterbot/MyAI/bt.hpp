#include <vector>
#include <functional>
#include <iostream>
#include <memory>

// #define debug

namespace bt {

enum state {
    success = 0,
    failure = 1,
    running = 2
};

struct node {
    virtual void start() {}
    virtual state step() = 0;
    virtual std::string type() = 0;
    virtual void print() {
        std::cerr << type();
    }
    virtual void printStack() {
        print();
    }

    
    virtual ~node() {}

    void run() {
        start();
        for (;;) {
            if (step() == state::success) {
                break;
            }
        }
    }
};


#define DECLARE_STR_TYPE(classname) \
std::string type() { return #classname; } 

struct action_node : public node {
    DECLARE_STR_TYPE(action_node)

    action_node(std::function<void()> f): f(f) {}
    state step() {
        #ifdef debug
            std::cerr << type() << std::endl;
        #endif
        f();
        return state::success;
    }
    std::function<void()> f;
};

struct action_once_node : public node {
    DECLARE_STR_TYPE(action_once_node)

    action_once_node(std::function<void()> f): f(f) {}
    
    void start() {
        done = false;
    }
    state step() {
        #ifdef debug
            std::cerr << type() << std::endl;
        #endif
        if (done)
            return state::success;
        f();
        done = true;
        return state::success;
    }
    std::function<void()> f;
    bool done = false;
};

struct wait_until_node : public node {
    DECLARE_STR_TYPE(wait_until_node)

    wait_until_node(std::function<bool()> f): f(f) {}
    void start() {
        done = false;
    }
    state step() {
        #ifdef debug
            std::cerr << type() << std::endl;
        #endif
        if (done || f()) {
            done = true;
            return state::success;
        }
        return state::running;
    }
    std::function<bool()> f;
    bool done = false;
};


struct sequencer_node : public node {
    DECLARE_STR_TYPE(sequencer_node)
    
    sequencer_node(std::vector<std::shared_ptr<node>>&& nodes): nodes{std::move(nodes)} {}
    void print() {
        std::cerr << type() << '(';
        for (auto& n : nodes) {
            n->print();
            std::cerr << " -> ";
        }
        std::cerr << ')';
    }

    void printStack() {
        std::cerr << type() << ' ' << current_idx << ' ';
        nodes[current_idx]->printStack();
    }
    
    void start() {
        for (auto& node : nodes) {
            node->start();
        }
        current_idx = 0;
    }
    state step() {
        #ifdef debug
            std::cerr << type() << std::endl;
        #endif
        while (current_idx < nodes.size()) {
            auto node = nodes[current_idx];
            state s = node->step();
            if (s != state::success)
                return s;
            current_idx++;
        }
        return state::success;
    }
    std::vector<std::shared_ptr<node>> nodes;
    int current_idx{ 0 };
};

struct selector_node : public node {
    DECLARE_STR_TYPE(selector_node)

    selector_node(std::vector<std::shared_ptr<node>>&& nodes): nodes{std::move(nodes)} {}

    void print() {
        std::cerr << type() << '(';
        for (auto& n : nodes) {
            n->print();
            std::cerr << " | ";
        }
        std::cerr << ')';
    }
    void printStack() {
        std::cerr << type() << ' ' << current_idx << ' ';
        nodes[current_idx]->printStack();
    }

    void start() {
        for (auto& node : nodes) {
            node->start();
        }
        current_idx = 0;
    }
    state step() {
        #ifdef debug
            std::cerr << type() << std::endl;
        #endif
        while (current_idx < nodes.size()) {
            auto node = nodes[current_idx];
            state s = node->step();
            if (s != state::failure)
                return s;
            current_idx++;
        }
        return state::failure;
    }
    std::vector<std::shared_ptr<node>> nodes;
    int current_idx{ 0 };
};

struct condition_node : public node {
    condition_node(std::function<bool()> f): f(f) {}
    std::string type() { return "condition_node"; }

    void start() {}
    state step() {
        #ifdef debug
            std::cerr << type() << std::endl;
        #endif
        if (f()) {
            return state::success;
        }
        return state::failure;
    }
    std::function<bool()> f;
};


struct repeater_node : public node {
    repeater_node(std::shared_ptr<node> node): node(node) {}
    std::string type() { return "repeater_node"; }
    void print() {
        std::cerr << type() << '(';
        node->print();
        std::cerr << ')';
    }
    void start() {
        node->start();
    }
    state step() {
        #ifdef debug
            std::cerr << type() << std::endl;
        #endif
        if (node->step() != state::running) {
            node->start();
        }
        return state::running;
    }
    std::shared_ptr<node> node;
};


std::shared_ptr<node> action(std::function<void()> f) {
    return std::make_shared<action_node>(std::move(f));
}

std::shared_ptr<node> once(std::function<void()> f) {
    return std::make_shared<action_once_node>(std::move(f));
}

std::shared_ptr<node> wait_until(std::function<bool()> f) {
    return std::make_shared<wait_until_node>(std::move(f));
}

std::shared_ptr<node> repeat_until_success(std::function<bool()> f) {
    return wait_until(std::move(f));
}

std::shared_ptr<node> if_true(std::function<bool()> f) {
    return std::make_shared<condition_node>(std::move(f));
}

std::shared_ptr<node> sequence(std::vector<std::shared_ptr<node>>&& nodes) {
    return std::make_shared<sequencer_node>(std::move(nodes));
}

std::shared_ptr<node> repeat(std::shared_ptr<node> node) {
    return std::make_shared<repeater_node>(std::move(node));
}

std::shared_ptr<node> one_of(std::vector<std::shared_ptr<node>>&& nodes) {
    return std::make_shared<selector_node>(std::move(nodes));
}

}
