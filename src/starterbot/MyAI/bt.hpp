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
    virtual state update() = 0;
    virtual ~node() {}

    void run() {
        start();
        for (;;) {
            if (update() == state::success) {
                break;
            }
        }
    }
};

struct action_node : public node {
    action_node(std::function<void()> f): f(f) {}
    state update() {
        #ifdef debug
            std::cerr << "action_node" << std::endl;
        #endif
        f();
        return state::success;
    }
    std::function<void()> f;
};

struct action_once_node : public node {
    action_once_node(std::function<void()> f): f(f) {}
    void start() {
        done = false;
    }
    state update() {
        #ifdef debug
            std::cerr << "action_once_node" << std::endl;
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
    wait_until_node(std::function<bool()> f): f(f) {}
    void start() {
        done = false;
    }
    state update() {
        #ifdef debug
            std::cerr << "wait_until_node" << std::endl;
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
    sequencer_node(std::initializer_list<std::shared_ptr<node>>&& nodes): nodes{std::move(nodes)} {}
    void start() {
        for (auto& node : nodes) {
            node->start();
        }
    }
    state update() {
        #ifdef debug
            std::cerr << "sequencer_node" << std::endl;
        #endif
        for (auto& node : nodes) {
            state s = node->update();
            if (s != state::success)
                return s;
        }
        return state::success;
    }
    std::vector<std::shared_ptr<node>> nodes;
};

struct selector_node : public node {
    selector_node(std::initializer_list<std::shared_ptr<node>>&& nodes): nodes{std::move(nodes)} {}
    void start() {
        for (auto& node : nodes) {
            node->start();
        }
    }
    state update() {
        #ifdef debug
            std::cerr << "selector_node" << std::endl;
        #endif
        for (auto& node : nodes) {
            state s = node->update();
            if (s == state::success)
                return s;
        }
        return state::failure;
    }
    std::vector<std::shared_ptr<node>> nodes;
};

struct condition_node : public node {
    condition_node(std::function<bool()> f): f(f) {}
    void start() {}
    state update() {
        #ifdef debug
            std::cerr << "condition_node" << std::endl;
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
    void start() {
        node->start();
    }
    state update() {
        #ifdef debug
            std::cerr << "repeater_node" << std::endl;
        #endif
        if (node->update() != state::running) {
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

std::shared_ptr<node> if_true(std::function<bool()> f) {
    return std::make_shared<condition_node>(std::move(f));
}

std::shared_ptr<node> sequence(std::initializer_list<std::shared_ptr<node>>&& nodes) {
    return std::make_shared<sequencer_node>(std::move(nodes));
}

std::shared_ptr<node> repeat(std::shared_ptr<node> node) {
    return std::make_shared<repeater_node>(std::move(node));
}

std::shared_ptr<node> one_of(std::initializer_list<std::shared_ptr<node>>&& nodes) {
    return std::make_shared<selector_node>(std::move(nodes));
}

}
