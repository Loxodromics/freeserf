#include "scripted-agent.h"
#include "ai-logger.h"
#include <algorithm>
#include <cmath>
#include <random>

ScriptedAgent::ScriptedAgent(int difficulty, int personality, const std::string& name)
    : current_state(AgentState::NEED_CASTLE)
    , difficulty(difficulty)
    , personality(personality)
    , agent_name(name)
    , castle_position(0)
    , forester_position(0)
    , lumberjack_position(0)
    , decision_cooldown(10)  // Wait 10 ticks between decisions
    , last_action_tick(0) {
}

std::vector<AIAction> ScriptedAgent::get_actions(const GameState& state) {
    // Update agent state based on current game situation
    update_agent_state(state);
    
    // Check if we should take action (cooldown mechanism)
    if (!should_take_action(state)) {
        return {AIAction::no_action()};
    }
    
    // Update last action tick
    last_action_tick = state.game_tick;
    
    // Decide actions based on current state
    std::vector<AIAction> actions;
    
    switch (current_state) {
        case AgentState::NEED_CASTLE:
            actions = decide_castle_placement(state);
            break;
            
        case AgentState::NEED_FORESTER:
            actions = decide_forester_placement(state);
            break;
            
        case AgentState::NEED_LUMBERJACK:
            actions = decide_lumberjack_placement(state);
            break;
            
        case AgentState::NEED_ROADS:
            actions = decide_road_construction(state);
            break;
            
        case AgentState::PRODUCING:
            actions = decide_production_phase(state);
            break;
            
        case AgentState::EXPANDING:
            actions = decide_expansion_phase(state);
            break;
    }
    
    // Return no action if no suitable actions found
    if (actions.empty()) {
        return {AIAction::no_action()};
    }
    
    return actions;
}

int ScriptedAgent::get_action_space_size() const {
    // Phase 1 has 5 main actions + 2 special actions
    return 7;
}

std::vector<bool> ScriptedAgent::get_valid_actions(const GameState& state) {
    // For simplicity, assume all actions are potentially valid
    // A more sophisticated implementation would check specific constraints
    return std::vector<bool>(get_action_space_size(), true);
}

void ScriptedAgent::set_difficulty(int new_difficulty) {
    difficulty = std::max(0, std::min(10, new_difficulty));
}

void ScriptedAgent::set_personality(int new_personality) {
    personality = std::max(0, std::min(10, new_personality));
}

AgentType ScriptedAgent::get_agent_type() const {
    return AgentType::SCRIPTED;
}

std::string ScriptedAgent::get_agent_name() const {
    return agent_name;
}

bool ScriptedAgent::is_ready() const {
    return true;  // ScriptedAgent is always ready
}

// State machine decision methods
std::vector<AIAction> ScriptedAgent::decide_castle_placement(const GameState& state) {
    // If we already have a castle, move to next state
    if (state.self.has_castle) {
        return {AIAction::no_action()};
    }
    
    MapPos castle_pos = find_best_castle_position(state);
    if (castle_pos != 0) {
        castle_position = castle_pos;
        AIAction action = AIAction::build_castle(castle_pos, 1.0f);
        return {action};
    }
    
    return {AIAction::no_action()};
}

std::vector<AIAction> ScriptedAgent::decide_forester_placement(const GameState& state) {
    // Check if we already have a forester
    if (state.self.building_counts[static_cast<int>(Building::TypeForester)] > 0) {
        return {AIAction::no_action()};
    }
    
    MapPos forester_pos = find_forest_position_near(castle_position, state);
    if (forester_pos != 0) {
        forester_position = forester_pos;
        AIAction action = AIAction::build_forester(forester_pos, 0.8f);
        return {action};
    }
    
    return {AIAction::no_action()};
}

std::vector<AIAction> ScriptedAgent::decide_lumberjack_placement(const GameState& state) {
    // Check if we already have a lumberjack
    if (state.self.building_counts[static_cast<int>(Building::TypeLumberjack)] > 0) {
        return {AIAction::no_action()};
    }
    
    // Place lumberjack near forester or castle
    MapPos reference_pos = (forester_position != 0) ? forester_position : castle_position;
    MapPos lumberjack_pos = find_building_position_near(reference_pos, state);
    
    if (lumberjack_pos != 0) {
        lumberjack_position = lumberjack_pos;
        AIAction action = AIAction::build_lumberjack(lumberjack_pos, 0.7f);
        return {action};
    }
    
    return {AIAction::no_action()};
}

std::vector<AIAction> ScriptedAgent::decide_road_construction(const GameState& state) {
    // Simple road construction: connect buildings to castle
    std::vector<MapPos> buildings;
    
    if (forester_position != 0) buildings.push_back(forester_position);
    if (lumberjack_position != 0) buildings.push_back(lumberjack_position);
    
    // Try to build road from castle to the nearest building without road
    for (MapPos building_pos : buildings) {
        if (building_pos != castle_position) {
            AIAction action = AIAction::build_road(castle_position, building_pos, 0.6f);
            return {action};
        }
    }
    
    return {AIAction::no_action()};
}

std::vector<AIAction> ScriptedAgent::decide_production_phase(const GameState& state) {
    // In production phase, occasionally expand or build more infrastructure
    // For now, just wait and monitor
    return {AIAction::no_action()};
}

std::vector<AIAction> ScriptedAgent::decide_expansion_phase(const GameState& state) {
    // In expansion phase, look for new locations to build
    // For Phase 1, just wait
    return {AIAction::no_action()};
}

// Position finding algorithms
MapPos ScriptedAgent::find_best_castle_position(const GameState& state) {
    const auto& map = state.map;
    
    // Look for a position that's:
    // 1. Not too close to edges
    // 2. On suitable terrain
    // 3. Has some nearby resources
    
    int best_score = -1;
    MapPos best_pos = 0;
    
    for (int y = 10; y < map.height - 10; y += 2) {
        for (int x = 10; x < map.width - 10; x += 2) {
            MapPos pos = y * map.width + x;
            
            if (is_position_suitable_for_castle(pos, state)) {
                int score = 100;  // Base score
                
                // Prefer positions with nearby trees
                score += count_trees_near(pos, state, 5) * 2;
                
                // Prefer positions not too close to water
                // Add small randomization based on personality
                score += (personality % 3) * 10;
                
                if (score > best_score) {
                    best_score = score;
                    best_pos = pos;
                }
            }
        }
    }
    
    return best_pos;
}

MapPos ScriptedAgent::find_forest_position_near(MapPos center, const GameState& state) {
    const auto& map = state.map;
    
    // Search in expanding radius around center
    for (int radius = 2; radius <= 10; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx * dx + dy * dy > radius * radius) continue;
                
                int x = (center % map.width) + dx;
                int y = (center / map.width) + dy;
                
                if (x < 0 || x >= map.width || y < 0 || y >= map.height) continue;
                
                MapPos pos = y * map.width + x;
                
                if (is_position_suitable_for_building(pos, state)) {
                    // Good location if it has trees nearby
                    if (count_trees_near(pos, state, 3) >= 2) {
                        return pos;
                    }
                }
            }
        }
    }
    
    return 0;  // No suitable position found
}

MapPos ScriptedAgent::find_building_position_near(MapPos center, const GameState& state) {
    const auto& map = state.map;
    
    // Search in expanding radius around center
    for (int radius = 2; radius <= 8; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx * dx + dy * dy > radius * radius) continue;
                
                int x = (center % map.width) + dx;
                int y = (center / map.width) + dy;
                
                if (x < 0 || x >= map.width || y < 0 || y >= map.height) continue;
                
                MapPos pos = y * map.width + x;
                
                if (is_position_suitable_for_building(pos, state)) {
                    return pos;
                }
            }
        }
    }
    
    return 0;  // No suitable position found
}

std::vector<MapPos> ScriptedAgent::plan_road_between(MapPos from, MapPos to, const GameState& state) {
    // Simple road planning - just return direct connection
    // A more sophisticated implementation would use pathfinding
    return {from, to};
}

// Helper methods
bool ScriptedAgent::has_building_at(MapPos pos, const GameState& state) {
    return state.map.has_building[pos];
}

bool ScriptedAgent::is_position_suitable_for_castle(MapPos pos, const GameState& state) {
    const auto& map = state.map;
    
    // Check if position is within map bounds
    if (pos >= map.terrain_types.size()) return false;
    
    // Check if position already has building
    if (map.has_building[pos]) return false;
    
    // Check terrain type (simplified check)
    uint8_t terrain = map.terrain_types[pos];
    
    // Avoid water and very steep terrain (basic heuristic)
    return terrain < 8;  // Simplified terrain check
}

bool ScriptedAgent::is_position_suitable_for_building(MapPos pos, const GameState& state) {
    const auto& map = state.map;
    
    // Check if position is within map bounds
    if (pos >= map.terrain_types.size()) return false;
    
    // Check if position already has building or flag
    if (map.has_building[pos] || map.has_flag[pos]) return false;
    
    // Check if we own this territory
    if (map.ownership[pos] != state.self.player_index) return false;
    
    // Check terrain type
    uint8_t terrain = map.terrain_types[pos];
    
    // Basic terrain suitability check
    return terrain < 8;  // Simplified terrain check
}

int ScriptedAgent::count_trees_near(MapPos pos, const GameState& state, int radius) {
    const auto& map = state.map;
    int tree_count = 0;
    
    int x_center = pos % map.width;
    int y_center = pos / map.width;
    
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx * dx + dy * dy > radius * radius) continue;
            
            int x = x_center + dx;
            int y = y_center + dy;
            
            if (x < 0 || x >= map.width || y < 0 || y >= map.height) continue;
            
            MapPos check_pos = y * map.width + x;
            
            // Check terrain type for trees (simplified)
            uint8_t terrain = map.terrain_types[check_pos];
            if (terrain >= 4 && terrain <= 7) {  // Forest terrain types (heuristic)
                tree_count++;
            }
        }
    }
    
    return tree_count;
}

float ScriptedAgent::calculate_action_priority(const AIAction& action, const GameState& state) {
    // Simple priority calculation based on action type and current needs
    switch (action.type) {
        case AIActionType::BUILD_CASTLE:
            return 1.0f;  // Highest priority
        case AIActionType::BUILD_FORESTER:
            return 0.8f;
        case AIActionType::BUILD_LUMBERJACK:
            return 0.7f;
        case AIActionType::BUILD_ROAD:
            return 0.6f;
        case AIActionType::BUILD_FLAG:
            return 0.5f;
        default:
            return 0.0f;
    }
}

// State management
void ScriptedAgent::update_agent_state(const GameState& state) {
    std::string old_state = get_state_name();
    
    // State transition logic
    switch (current_state) {
        case AgentState::NEED_CASTLE:
            if (state.self.has_castle) {
                current_state = AgentState::NEED_FORESTER;
                castle_position = 0;  // Will be updated when we know castle location
                
                // Find castle position from building list
                for (size_t i = 0; i < state.self.building_positions.size(); i++) {
                    if (state.self.building_types[i] == Building::TypeCastle) {
                        castle_position = state.self.building_positions[i];
                        break;
                    }
                }
            }
            break;
            
        case AgentState::NEED_FORESTER:
            if (state.self.building_counts[static_cast<int>(Building::TypeForester)] > 0) {
                current_state = AgentState::NEED_LUMBERJACK;
            }
            break;
            
        case AgentState::NEED_LUMBERJACK:
            if (state.self.building_counts[static_cast<int>(Building::TypeLumberjack)] > 0) {
                current_state = AgentState::NEED_ROADS;
            }
            break;
            
        case AgentState::NEED_ROADS:
            // Transition to producing after some time or when basic infrastructure is connected
            if (state.game_tick > 500) {  // After some time
                current_state = AgentState::PRODUCING;
            }
            break;
            
        case AgentState::PRODUCING:
            // Transition to expanding when we have good production
            if (state.self.resource_counts[6] > 5) {  // If we have some logs
                current_state = AgentState::EXPANDING;
            }
            break;
            
        case AgentState::EXPANDING:
            // Stay in expanding state
            break;
    }
    
    // Log state changes
    std::string new_state = get_state_name();
    if (old_state != new_state) {
        AILogger::log_agent_state_change(state.self.player_index, old_state, new_state);
    }
}

std::string ScriptedAgent::get_state_name() const {
    switch (current_state) {
        case AgentState::NEED_CASTLE: return "NEED_CASTLE";
        case AgentState::NEED_FORESTER: return "NEED_FORESTER";
        case AgentState::NEED_LUMBERJACK: return "NEED_LUMBERJACK";
        case AgentState::NEED_ROADS: return "NEED_ROADS";
        case AgentState::PRODUCING: return "PRODUCING";
        case AgentState::EXPANDING: return "EXPANDING";
        default: return "UNKNOWN";
    }
}

bool ScriptedAgent::should_take_action(const GameState& state) {
    // Simple cooldown mechanism
    return (state.game_tick - last_action_tick) >= decision_cooldown;
}