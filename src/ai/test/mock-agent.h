#pragma once

#include "../agent.h"
#include "../ai-action.h"
#include <queue>

/**
 * Mock Agent for testing
 * Allows controllable, predictable behavior for unit tests
 */
class MockAgent : public Agent {
private:
    std::queue<std::vector<AIAction>> action_queue;
    std::vector<bool> valid_actions_mask;
    std::string agent_name;
    bool ready_state;
    
public:
    explicit MockAgent(const std::string& name = "MockAgent");
    
    // Agent interface implementation
    std::vector<AIAction> get_actions(const GameState& state) override;
    int get_action_space_size() const override;
    std::vector<bool> get_valid_actions(const GameState& state) override;
    void set_difficulty(int difficulty) override;
    void set_personality(int personality) override;
    AgentType get_agent_type() const override;
    std::string get_agent_name() const override;
    bool is_ready() const override;
    
    // Test control methods
    void queue_actions(const std::vector<AIAction>& actions);
    void set_valid_actions_mask(const std::vector<bool>& mask);
    void set_ready_state(bool ready);
    void clear_action_queue();
    
    // Test inspection methods
    size_t get_queued_action_count() const;
    bool has_queued_actions() const;
};