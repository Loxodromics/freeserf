#include "mock-agent.h"

MockAgent::MockAgent(const std::string& name) 
    : agent_name(name), ready_state(true) {
}

std::vector<AIAction> MockAgent::get_actions(const GameState& state) {
    if (action_queue.empty()) {
        // Return no action if queue is empty
        return {AIAction::no_action()};
    }
    
    std::vector<AIAction> actions = action_queue.front();
    action_queue.pop();
    return actions;
}

int MockAgent::get_action_space_size() const {
    // Phase 1 has 5 main actions + 2 special actions
    return 7;
}

std::vector<bool> MockAgent::get_valid_actions(const GameState& state) {
    if (!valid_actions_mask.empty()) {
        return valid_actions_mask;
    }
    
    // Default: all actions are valid
    return std::vector<bool>(get_action_space_size(), true);
}

void MockAgent::set_difficulty(int difficulty) {
    // MockAgent ignores difficulty settings
}

void MockAgent::set_personality(int personality) {
    // MockAgent ignores personality settings
}

AgentType MockAgent::get_agent_type() const {
    return AgentType::SCRIPTED;  // MockAgent pretends to be scripted
}

std::string MockAgent::get_agent_name() const {
    return agent_name;
}

bool MockAgent::is_ready() const {
    return ready_state;
}

void MockAgent::queue_actions(const std::vector<AIAction>& actions) {
    action_queue.push(actions);
}

void MockAgent::set_valid_actions_mask(const std::vector<bool>& mask) {
    valid_actions_mask = mask;
}

void MockAgent::set_ready_state(bool ready) {
    ready_state = ready;
}

void MockAgent::clear_action_queue() {
    while (!action_queue.empty()) {
        action_queue.pop();
    }
}

size_t MockAgent::get_queued_action_count() const {
    return action_queue.size();
}

bool MockAgent::has_queued_actions() const {
    return !action_queue.empty();
}