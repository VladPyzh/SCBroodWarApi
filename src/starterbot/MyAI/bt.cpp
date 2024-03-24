#include "bt.hpp"
namespace bt {
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

    std::shared_ptr<node> repeat_node_until_success(std::shared_ptr<node> node) {
        return std::make_shared<repeate_until_node>(std::move(node));
    }

    std::shared_ptr<node> if_true(std::function<bool()> f) {
        return std::make_shared<condition_node>(std::move(f));
    }

    std::shared_ptr<node> try_node(std::shared_ptr<node> node) {
        return std::make_shared<try_node_node>(std::move(node));
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

    std::shared_ptr<node> fail() {
        return if_true([]() { return false; });
    }
}