#pragma once

#include "agent.h"
#include "ai-action.h"
#include "game-state.h"
#include <vector>
#include <string>

// Forward declarations
class Game;
class Player;

/**
 * ScriptedAgent - A functional AI agent that replaces MockAgent
 * Implements basic decision-making for Phase 1 actions with a simple state machine
 */
class ScriptedAgent : public Agent {
private:
    enum class AgentState {
        NEED_CASTLE,
        NEED_FORESTER,
        NEED_LUMBERJACK,
        NEED_ROADS,
        PRODUCING,
        EXPANDING
    };
    
    AgentState current_state;
    int difficulty;
    int personality;
    std::string agent_name;
    
    // Phase 1 strategy data
    MapPos castle_position;
    MapPos forester_position;
    MapPos lumberjack_position;
    std::vector<MapPos> planned_roads;
    
    // Decision cooldown (prevent spamming actions)
    int decision_cooldown;
    int last_action_tick;
    
public:
    explicit ScriptedAgent(int difficulty = 5, int personality = 0, const std::string& name = "ScriptedAgent");
    
    // Agent interface implementation
    std::vector<AIAction> get_actions(const GameState& state) override;
    // Enhanced version with Game and Player access for authoritative validation
    std::vector<AIAction> get_actions(const GameState& state, Game* game, Player* player);
    int get_action_space_size() const override;
    std::vector<bool> get_valid_actions(const GameState& state) override;
    void set_difficulty(int difficulty) override;
    void set_personality(int personality) override;
    AgentType get_agent_type() const override;
    std::string get_agent_name() const override;
    bool is_ready() const override;
    
    // Training support (not used for scripted agent)
    void receive_reward(float reward) override {}
    void episode_started(const GameState& initial_state) override {}
    void episode_ended(bool victory, float final_score) override {}
    
private:
    // State machine decision methods
    std::vector<AIAction> decide_castle_placement(const GameState& state, Game* game = nullptr, Player* player = nullptr);
    std::vector<AIAction> decide_forester_placement(const GameState& state, Game* game = nullptr, Player* player = nullptr);
    std::vector<AIAction> decide_lumberjack_placement(const GameState& state, Game* game = nullptr, Player* player = nullptr);
    std::vector<AIAction> decide_road_construction(const GameState& state);
    std::vector<AIAction> decide_flag_placement(const GameState& state, Game* game = nullptr, Player* player = nullptr);
    std::vector<AIAction> decide_production_phase(const GameState& state);
    std::vector<AIAction> decide_expansion_phase(const GameState& state);
    
    // Position finding algorithms
    MapPos find_best_castle_position(const GameState& state, Game* game = nullptr, Player* player = nullptr);
    MapPos find_forest_position_near(MapPos center, const GameState& state, Game* game = nullptr, Player* player = nullptr);
    MapPos find_building_position_near(MapPos center, const GameState& state, Game* game = nullptr, Player* player = nullptr);
    std::vector<MapPos> plan_road_between(MapPos from, MapPos to, const GameState& state);
    
    // Castle position finding helpers
    MapPos find_castle_position_with_game_validation(const GameState& state, Game* game, Player* player);
    MapPos find_castle_position_fallback(const GameState& state);
    
    // Building position finding helpers
    MapPos find_forest_position_with_game_validation(MapPos center, const GameState& state, Game* game, Player* player);
    MapPos find_forest_position_fallback(MapPos center, const GameState& state);
    MapPos find_building_position_with_game_validation(MapPos center, const GameState& state, Building::Type type, Game* game, Player* player);
    MapPos find_building_position_fallback(MapPos center, const GameState& state);
    
    // Flag position finding helpers
    MapPos find_flag_position_near(MapPos building_pos, const GameState& state, Game* game = nullptr, Player* player = nullptr);
    
    // Helper methods
    bool has_building_at(MapPos pos, const GameState& state);
    bool is_position_suitable_for_castle(MapPos pos, const GameState& state);
    bool is_position_suitable_for_building(MapPos pos, const GameState& state);
    int count_trees_near(MapPos pos, const GameState& state, int radius = 3);
    float calculate_action_priority(const AIAction& action, const GameState& state);
    
    // State management
    void update_agent_state(const GameState& state);
    std::string get_state_name() const;
    bool should_take_action(const GameState& state);
};