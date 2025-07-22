#pragma once

#include <vector>
#include <memory>
#include <string>

// Forward declarations
struct GameState;
struct AIAction;
class Player;
class Game;

enum class AgentType {
    SCRIPTED,
    NEURAL_NETWORK,
    HUMAN_ASSISTED,
    RANDOM
};

/**
 * Main Agent Interface
 * All AI implementations must inherit from this interface
 */
class Agent {
public:
    virtual ~Agent() = default;
    
    // Core AI decision-making
    virtual std::vector<AIAction> get_actions(const GameState& state) = 0;
    
    // Action space information (required for ML agents)
    virtual int get_action_space_size() const = 0;
    virtual std::vector<bool> get_valid_actions(const GameState& state) = 0;
    
    // Agent configuration
    virtual void set_difficulty(int difficulty) = 0;  // 0-10 scale
    virtual void set_personality(int personality) = 0;  // 0-10 different personalities
    virtual AgentType get_agent_type() const = 0;
    
    // Performance and debugging
    virtual std::string get_agent_name() const = 0;
    virtual bool is_ready() const = 0;  // Can the agent make decisions?
    
    // Training support (optional - only used by ML agents)
    virtual void receive_reward(float reward) {}
    virtual void episode_started(const GameState& initial_state) {}
    virtual void episode_ended(bool victory, float final_score) {}
};