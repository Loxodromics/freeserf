#pragma once

#include "agent.h"
#include "ai-action.h"
#include "game-state.h"
#include "../building.h"
#include <random>
#include <vector>

class RandomAgent : public Agent {
private:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> prob_dist;
    std::string agent_name;
    int difficulty_level;
    int personality_type;
    
    // Available building types for random selection
    std::vector<Building::Type> available_buildings;
    
    // Random selection parameters
    static constexpr float BASE_BUILDING_PROBABILITY = 0.3f;  // 30% chance per tick
    static constexpr float BASE_ROAD_PROBABILITY = 0.2f;      // 20% chance per tick
    static constexpr int MAX_ACTIONS_PER_TICK = 2;            // Limit actions to avoid spam

public:
    RandomAgent(const std::string& name = "RandomAgent");
    virtual ~RandomAgent() = default;
    
    // Core AI decision-making
    virtual std::vector<AIAction> get_actions(const GameState& state) override;
    
    // Action space information
    virtual int get_action_space_size() const override;
    virtual std::vector<bool> get_valid_actions(const GameState& state) override;
    
    // Agent configuration
    virtual void set_difficulty(int difficulty) override;
    virtual void set_personality(int personality) override;
    virtual AgentType get_agent_type() const override;
    
    // Performance and debugging
    virtual std::string get_agent_name() const override;
    virtual bool is_ready() const override;

private:
    void initialize_building_types();
    bool should_place_building(const GameState& state);
    bool should_build_roads(const GameState& state);
    Building::Type get_random_building_type();
    MapPos get_random_position(const GameState& state);
    std::vector<MapPos> get_owned_positions(const GameState& state);
    std::vector<MapPos> get_available_flags(const GameState& state);
    AIActionType building_type_to_action_type(Building::Type type);
    
    // Flag discovery system for immediate road building
    MapPos calculate_building_flag_position(MapPos building_pos, Building::Type type, const GameState& state);
    std::vector<MapPos> find_all_player_flags(const GameState& state);
    MapPos find_castle_flag(const GameState& state);
    MapPos find_nearest_flag(const GameState& state, MapPos target_pos);
    int calculate_distance(MapPos pos1, MapPos pos2, const GameState& state);
    MapPos find_connection_target_flag(const GameState& state, MapPos new_flag_pos);
    
    // Robust castle flag detection system
    MapPos find_actual_flag_near_position(MapPos center, const GameState& state, int radius);
    bool is_castle_connected(const GameState& state);
    int count_castle_connections(const GameState& state);
};