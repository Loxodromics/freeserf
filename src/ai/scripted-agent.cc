#include "scripted-agent.h"
#include "ai-logger.h"
#include "../game.h"
#include "../player.h"
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
    // Delegate to enhanced version without Game/Player access (fallback)
    return get_actions(state, nullptr, nullptr);
}

std::vector<AIAction> ScriptedAgent::get_actions(const GameState& state, Game* game, Player* player) {
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
            actions = decide_castle_placement(state, game, player);
            break;
            
        case AgentState::NEED_FORESTER:
            actions = decide_forester_placement(state, game, player);
            break;
            
        case AgentState::NEED_LUMBERJACK:
            actions = decide_lumberjack_placement(state, game, player);
            break;
            
        case AgentState::NEED_ROADS:
            actions = decide_road_construction(state, game, player);
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
std::vector<AIAction> ScriptedAgent::decide_castle_placement(const GameState& state, Game* game, Player* player) {
    // If we already have a castle, move to next state
    if (state.self.has_castle) {
        return {AIAction::no_action()};
    }
    
    MapPos castle_pos = find_best_castle_position(state, game, player);
    if (castle_pos != 0) {
        castle_position = castle_pos;
        AIAction action = AIAction::build_castle(castle_pos, 1.0f);
        return {action};
    }
    
    return {AIAction::no_action()};
}

std::vector<AIAction> ScriptedAgent::decide_forester_placement(const GameState& state, Game* game, Player* player) {
    // Check if we already have a forester (completed + under construction)
    int forester_count = state.self.building_counts[static_cast<int>(Building::TypeForester)];
    AILogger::log_debug("Forester decision: current count = " + std::to_string(forester_count));
    
    if (forester_count > 0) {
        AILogger::log_debug("Forester decision: already have " + std::to_string(forester_count) + " foresters, skipping");
        return {AIAction::no_action()};
    }
    
    MapPos forester_pos = find_forest_position_near(castle_position, state, game, player);
    if (forester_pos != 0) {
        forester_position = forester_pos;
        AIAction action = AIAction::build_forester(forester_pos, 0.8f);
        return {action};
    }
    
    return {AIAction::no_action()};
}

std::vector<AIAction> ScriptedAgent::decide_lumberjack_placement(const GameState& state, Game* game, Player* player) {
    // Check if we already have a lumberjack (completed + under construction)
    int lumberjack_count = state.self.building_counts[static_cast<int>(Building::TypeLumberjack)];
    AILogger::log_debug("Lumberjack decision: current count = " + std::to_string(lumberjack_count));
    
    if (lumberjack_count > 0) {
        AILogger::log_debug("Lumberjack decision: already have " + std::to_string(lumberjack_count) + " lumberjacks, skipping");
        return {AIAction::no_action()};
    }
    
    // Place lumberjack near forester or castle
    MapPos reference_pos = (forester_position != 0) ? forester_position : castle_position;
    MapPos lumberjack_pos = find_building_position_near(reference_pos, state, game, player);
    
    if (lumberjack_pos != 0) {
        lumberjack_position = lumberjack_pos;
        AIAction action = AIAction::build_lumberjack(lumberjack_pos, 0.7f);
        return {action};
    }
    
    return {AIAction::no_action()};
}

std::vector<AIAction> ScriptedAgent::decide_flag_placement(const GameState& state, Game* game, Player* player) {
    AILogger::log_debug("Flag placement: checking buildings for flag needs");
    
    // Strategy: Place flags near buildings that don't have them yet
    // Priority order: castle -> forester -> lumberjack
    
    std::vector<std::pair<MapPos, std::string>> buildings_to_check;
    if (castle_position != 0) buildings_to_check.push_back({castle_position, "castle"});
    if (forester_position != 0) buildings_to_check.push_back({forester_position, "forester"});
    if (lumberjack_position != 0) buildings_to_check.push_back({lumberjack_position, "lumberjack"});
    
    for (const auto& building : buildings_to_check) {
        MapPos building_pos = building.first;
        const std::string& building_name = building.second;
        
        AILogger::log_debug("Flag placement: checking " + building_name + " at position " + std::to_string(building_pos));
        
        MapPos flag_pos = find_flag_position_near(building_pos, state, game, player);
        if (flag_pos != 0) {
            AILogger::log_debug("Flag placement: found position " + std::to_string(flag_pos) + " near " + building_name);
            AIAction action = AIAction::build_flag(flag_pos, 0.5f);
            return {action};
        }
    }
    
    AILogger::log_debug("Flag placement: no suitable flag positions found, transitioning to production");
    return {AIAction::no_action()};
}

std::vector<AIAction> ScriptedAgent::decide_road_construction(const GameState& state, Game* game, Player* player) {
    AILogger::log_debug("Road construction: starting road building phase");
    
    // Find castle flag position
    MapPos castle_flag = find_castle_flag_position(state, game, player);
    if (castle_flag == 0) {
        AILogger::log_debug("Road construction: castle flag not found, cannot build roads");
        return {AIAction::no_action()};
    }
    
    // Try to build roads from each building to castle (priority: forester first)
    
    // 1. Connect forester to castle
    if (forester_position != 0) {
        MapPos forester_flag = find_building_flag_position(Building::TypeForester, state, game, player);
        if (forester_flag != 0 && !road_exists_between(forester_flag, castle_flag, state, game, player)) {
            AILogger::log_debug("Road construction: building road forester_flag(" + std::to_string(forester_flag) + ") -> castle_flag(" + std::to_string(castle_flag) + ")");
            AIAction action = AIAction::build_road(forester_flag, castle_flag, 0.9f);
            return {action};
        }
    }
    
    // 2. Connect lumberjack to castle
    if (lumberjack_position != 0) {
        MapPos lumberjack_flag = find_building_flag_position(Building::TypeLumberjack, state, game, player);
        if (lumberjack_flag != 0 && !road_exists_between(lumberjack_flag, castle_flag, state, game, player)) {
            AILogger::log_debug("Road construction: building road lumberjack_flag(" + std::to_string(lumberjack_flag) + ") -> castle_flag(" + std::to_string(castle_flag) + ")");
            AIAction action = AIAction::build_road(lumberjack_flag, castle_flag, 0.9f);
            return {action};
        }
    }
    
    // All roads built or no buildings found
    AILogger::log_debug("Road construction: all roads complete, transitioning to production");
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
MapPos ScriptedAgent::find_best_castle_position(const GameState& state, Game* game, Player* player) {
    const auto& map = state.map;
    
    // Use authoritative game validation when available
    if (game != nullptr && player != nullptr) {
        return find_castle_position_with_game_validation(state, game, player);
    }
    
    // Fallback to simplified validation (legacy behavior)
    return find_castle_position_fallback(state);
}

// NEW: Authoritative castle position finding using game validation
MapPos ScriptedAgent::find_castle_position_with_game_validation(const GameState& state, Game* game, Player* player) {
    const auto& map = state.map;
    
    AILogger::log_debug("Castle search: Using authoritative game validation");
    AILogger::log_debug("Map: " + std::to_string(map.width) + "x" + std::to_string(map.height) + 
                       " (" + std::to_string(map.terrain_types.size()) + " positions)");
    
    // Calculate map center for center-outward search
    int center_x = map.width / 2;
    int center_y = map.height / 2;
    int max_radius = std::min(map.width, map.height) / 2 - 5; // Stay away from edges
    
    int positions_tested = 0;
    
    // Center-outward spiral search for efficiency
    for (int radius = 5; radius <= max_radius; radius += 3) {
        AILogger::log_debug("Testing radius " + std::to_string(radius));
        
        // Test positions in a circle around center
        for (int angle = 0; angle < 360; angle += 30) { // Every 30 degrees
            double rad = angle * 3.14159 / 180.0;
            int x = center_x + static_cast<int>(radius * std::cos(rad));
            int y = center_y + static_cast<int>(radius * std::sin(rad));
            
            // Bounds check
            if (x < 5 || x >= map.width - 5 || y < 5 || y >= map.height - 5) {
                continue;
            }
            
            // Use safe coordinate calculation
            size_t actual_map_size = map.terrain_types.size();
            int safe_width = static_cast<int>(std::sqrt(actual_map_size));
            MapPos pos = y * safe_width + x;
            
            if (pos >= actual_map_size) continue;
            
            positions_tested++;
            
            // Use authoritative game validation - no AI guessing!
            if (game->can_build_castle(pos, player)) {
                AILogger::log_debug("Castle found: pos=" + std::to_string(pos) + 
                                   " (" + std::to_string(x) + "," + std::to_string(y) + 
                                   ") radius=" + std::to_string(radius) + 
                                   " tested=" + std::to_string(positions_tested));
                return pos; // Return first valid position - no need to find "best"
            }
        }
        
        // Early termination if we've tested many positions
        if (positions_tested > 100) {
            AILogger::log_debug("Tested " + std::to_string(positions_tested) + " positions, expanding search...");
            break;
        }
    }
    
    // If spiral search failed, try a more systematic approach
    AILogger::log_debug("Spiral search failed, trying systematic search...");
    
    size_t actual_map_size = map.terrain_types.size();
    int safe_width = static_cast<int>(std::sqrt(actual_map_size));
    
    for (int y = 5; y < safe_width - 5; y += 4) {
        for (int x = 5; x < safe_width - 5; x += 4) {
            MapPos pos = y * safe_width + x;
            
            if (pos >= actual_map_size) continue;
            
            positions_tested++;
            
            if (game->can_build_castle(pos, player)) {
                AILogger::log_debug("Castle found (systematic): pos=" + std::to_string(pos) + 
                                   " (" + std::to_string(x) + "," + std::to_string(y) + 
                                   ") tested=" + std::to_string(positions_tested));
                return pos;
            }
            
            // Prevent excessive search time
            if (positions_tested > 500) {
                break;
            }
        }
        if (positions_tested > 500) break;
    }
    
    AILogger::log_debug("Castle search failed: tested " + std::to_string(positions_tested) + " positions");
    return 0; // No valid position found
}

// LEGACY: Fallback method using simplified validation (for compatibility)
MapPos ScriptedAgent::find_castle_position_fallback(const GameState& state) {
    const auto& map = state.map;
    
    AILogger::log_debug("Castle search: Using fallback validation (simplified)");
    
    size_t actual_map_size = map.terrain_types.size();
    if (actual_map_size == 0) {
        AILogger::log_debug("ERROR: Terrain vector is empty");
        return 0;
    }
    
    int safe_width = static_cast<int>(std::sqrt(actual_map_size));
    int positions_tested = 0;
    
    // Simple grid search as fallback
    for (int y = 10; y < safe_width - 10; y += 4) {
        for (int x = 10; x < safe_width - 10; x += 4) {
            MapPos pos = y * safe_width + x;
            positions_tested++;
            
            if (pos >= actual_map_size) continue;
            
            if (is_position_suitable_for_castle(pos, state)) {
                AILogger::log_debug("Castle found (fallback): pos=" + std::to_string(pos) + 
                                   " tested=" + std::to_string(positions_tested));
                return pos;
            }
            
            if (positions_tested > 100) break; // Limit search time
        }
        if (positions_tested > 100) break;
    }
    
    AILogger::log_debug("Fallback castle search failed: tested " + std::to_string(positions_tested) + " positions");
    return 0;
}

MapPos ScriptedAgent::find_forest_position_near(MapPos center, const GameState& state, Game* game, Player* player) {
    // Use authoritative game validation when available
    if (game != nullptr && player != nullptr) {
        return find_forest_position_with_game_validation(center, state, game, player);
    }
    
    // Fallback to simplified validation (legacy behavior)
    return find_forest_position_fallback(center, state);
}

// NEW: Authoritative forester position finding using game validation
MapPos ScriptedAgent::find_forest_position_with_game_validation(MapPos center, const GameState& state, Game* game, Player* player) {
    const auto& map = state.map;
    
    AILogger::log_debug("Forester search: Using authoritative game validation near position " + std::to_string(center));
    
    auto game_map = game->get_map();
    int map_cols = game_map->get_cols();
    int map_rows = game_map->get_rows();
    
    // Convert center position to coordinates
    int center_x = center % map_cols;
    int center_y = center / map_cols;
    
    int positions_tested = 0;
    const int max_positions = 80; // Limit search for performance
    
    // Search in expanding radius around center
    for (int radius = 2; radius <= 12 && positions_tested < max_positions; radius += 2) {
        for (int angle = 0; angle < 360 && positions_tested < max_positions; angle += 45) {
            double rad = angle * 3.14159 / 180.0;
            int x = center_x + static_cast<int>(radius * std::cos(rad));
            int y = center_y + static_cast<int>(radius * std::sin(rad));
            
            // Bounds check
            if (x < 2 || x >= map_cols - 2 || y < 2 || y >= map_rows - 2) {
                continue;
            }
            
            MapPos pos = y * map_cols + x;
            positions_tested++;
            
            // Use authoritative game validation for forester
            if (game->can_build_building(pos, Building::TypeForester, player)) {
                // Check if there are trees nearby for the forester to work with
                if (count_trees_near(pos, state, 3) >= 1) {
                    AILogger::log_debug("Forester found: pos=" + std::to_string(pos) + 
                                       " (" + std::to_string(x) + "," + std::to_string(y) + 
                                       ") radius=" + std::to_string(radius) + 
                                       " tested=" + std::to_string(positions_tested));
                    return pos;
                }
            }
        }
    }
    
    AILogger::log_debug("Forester search failed: tested " + std::to_string(positions_tested) + " positions");
    return 0; // No valid position found
}

// LEGACY: Fallback forester position finding using simplified validation
MapPos ScriptedAgent::find_forest_position_fallback(MapPos center, const GameState& state) {
    const auto& map = state.map;
    
    AILogger::log_debug("Forester search: Using fallback validation (simplified)");
    
    // Use safe dimensions like in castle position finding
    size_t actual_map_size = map.terrain_types.size();
    if (actual_map_size == 0) return 0;
    
    int safe_width = static_cast<int>(std::sqrt(actual_map_size));
    
    // Search in expanding radius around center
    for (int radius = 2; radius <= 10; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx * dx + dy * dy > radius * radius) continue;
                
                int x = (center % safe_width) + dx;
                int y = (center / safe_width) + dy;
                
                if (x < 0 || x >= safe_width || y < 0 || y >= safe_width) continue;
                
                MapPos pos = y * safe_width + x;
                
                // Bounds check
                if (pos >= actual_map_size) continue;
                
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

MapPos ScriptedAgent::find_building_position_near(MapPos center, const GameState& state, Game* game, Player* player) {
    // Use authoritative game validation when available
    if (game != nullptr && player != nullptr) {
        return find_building_position_with_game_validation(center, state, Building::TypeLumberjack, game, player);
    }
    
    // Fallback to simplified validation (legacy behavior)
    return find_building_position_fallback(center, state);
}

// NEW: Authoritative building position finding using game validation
MapPos ScriptedAgent::find_building_position_with_game_validation(MapPos center, const GameState& state, Building::Type type, Game* game, Player* player) {
    const auto& map = state.map;
    
    AILogger::log_debug("Building search: Using authoritative game validation for type " + std::to_string(type) + " near position " + std::to_string(center));
    
    auto game_map = game->get_map();
    int map_cols = game_map->get_cols();
    int map_rows = game_map->get_rows();
    
    // Convert center position to coordinates
    int center_x = center % map_cols;
    int center_y = center / map_cols;
    
    int positions_tested = 0;
    const int max_positions = 60; // Limit search for performance
    
    // Search in expanding radius around center
    for (int radius = 1; radius <= 10 && positions_tested < max_positions; radius += 2) {
        for (int angle = 0; angle < 360 && positions_tested < max_positions; angle += 60) {
            double rad = angle * 3.14159 / 180.0;
            int x = center_x + static_cast<int>(radius * std::cos(rad));
            int y = center_y + static_cast<int>(radius * std::sin(rad));
            
            // Bounds check
            if (x < 2 || x >= map_cols - 2 || y < 2 || y >= map_rows - 2) {
                continue;
            }
            
            MapPos pos = y * map_cols + x;
            positions_tested++;
            
            // Use authoritative game validation for building
            if (game->can_build_building(pos, type, player)) {
                AILogger::log_debug("Building found: pos=" + std::to_string(pos) + 
                                   " (type " + std::to_string(type) + ") " +
                                   " (" + std::to_string(x) + "," + std::to_string(y) + 
                                   ") radius=" + std::to_string(radius) + 
                                   " tested=" + std::to_string(positions_tested));
                return pos;
            }
        }
    }
    
    AILogger::log_debug("Building search failed for type " + std::to_string(type) + ": tested " + std::to_string(positions_tested) + " positions");
    return 0; // No valid position found
}

// LEGACY: Fallback building position finding using simplified validation
MapPos ScriptedAgent::find_building_position_fallback(MapPos center, const GameState& state) {
    const auto& map = state.map;
    
    AILogger::log_debug("Building search: Using fallback validation (simplified)");
    
    // Use safe dimensions
    size_t actual_map_size = map.terrain_types.size();
    if (actual_map_size == 0) return 0;
    
    int safe_width = static_cast<int>(std::sqrt(actual_map_size));
    
    // Search in expanding radius around center
    for (int radius = 2; radius <= 8; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx * dx + dy * dy > radius * radius) continue;
                
                int x = (center % safe_width) + dx;
                int y = (center / safe_width) + dy;
                
                if (x < 0 || x >= safe_width || y < 0 || y >= safe_width) continue;
                
                MapPos pos = y * safe_width + x;
                
                // Bounds check
                if (pos >= actual_map_size) continue;
                
                if (is_position_suitable_for_building(pos, state)) {
                    return pos;
                }
            }
        }
    }
    
    return 0;  // No suitable position found
}

MapPos ScriptedAgent::find_flag_position_near(MapPos building_pos, const GameState& state, Game* game, Player* player) {
    AILogger::log_debug("Flag search: finding position near building " + std::to_string(building_pos));
    
    // Use authoritative validation when available
    if (game != nullptr && player != nullptr) {
        auto game_map = game->get_map();
        int map_cols = game_map->get_cols();
        int map_rows = game_map->get_rows();
        
        // Convert building position to coordinates
        int building_x = building_pos % map_cols;
        int building_y = building_pos / map_cols;
        
        int positions_tested = 0;
        const int max_positions = 40; // Limit search for performance
        
        // Search in expanding radius around building (flags should be close)
        for (int radius = 1; radius <= 5 && positions_tested < max_positions; radius++) {
            for (int angle = 0; angle < 360 && positions_tested < max_positions; angle += 60) {
                double rad = angle * 3.14159 / 180.0;
                int x = building_x + static_cast<int>(radius * std::cos(rad));
                int y = building_y + static_cast<int>(radius * std::sin(rad));
                
                // Bounds check
                if (x < 1 || x >= map_cols - 1 || y < 1 || y >= map_rows - 1) {
                    continue;
                }
                
                MapPos pos = y * map_cols + x;
                positions_tested++;
                
                // Use authoritative game validation for flag
                if (game->can_build_flag(pos, player)) {
                    AILogger::log_debug("Flag found: pos=" + std::to_string(pos) + 
                                       " (" + std::to_string(x) + "," + std::to_string(y) + 
                                       ") radius=" + std::to_string(radius) + 
                                       " tested=" + std::to_string(positions_tested));
                    return pos;
                }
            }
        }
        
        AILogger::log_debug("Flag search failed: tested " + std::to_string(positions_tested) + " positions");
        return 0; // No valid position found
    }
    
    // Fallback for when Game/Player not available
    AILogger::log_debug("Flag search: using fallback (no game validation)");
    return 0;
}

MapPos ScriptedAgent::find_castle_flag_position(const GameState& state, Game* game, Player* player) {
    AILogger::log_debug("Castle flag search: looking for castle flag position");
    
    if (game != nullptr && castle_position != 0) {
        // Find castle building at stored position
        auto game_map = game->get_map();
        if (game_map->has_building(castle_position)) {
            // Get building from game
            Building* building = game->get_building_at_pos(castle_position);
            if (building && building->get_type() == Building::TypeCastle && building->get_owner() == player->get_index()) {
                unsigned int flag_index = building->get_flag_index();
                if (flag_index != 0) {
                    Flag* flag = game->get_flag(flag_index);
                    if (flag) {
                        MapPos flag_pos = flag->get_position();
                        AILogger::log_debug("Castle flag found at position " + std::to_string(flag_pos));
                        return flag_pos;
                    }
                }
            }
        }
    }
    
    AILogger::log_debug("Castle flag search failed");
    return 0;
}

MapPos ScriptedAgent::find_building_flag_position(Building::Type type, const GameState& state, Game* game, Player* player) {
    AILogger::log_debug("Building flag search: looking for " + std::to_string(type) + " flag position");
    
    if (game != nullptr && player != nullptr) {
        MapPos building_pos = 0;
        
        // Get the stored position for the building type
        switch (type) {
            case Building::TypeForester:
                building_pos = forester_position;
                break;
            case Building::TypeLumberjack:
                building_pos = lumberjack_position;
                break;
            default:
                AILogger::log_debug("Building flag search: unsupported building type " + std::to_string(type));
                return 0;
        }
        
        if (building_pos != 0) {
            auto game_map = game->get_map();
            if (game_map->has_building(building_pos)) {
                Building* building = game->get_building_at_pos(building_pos);
                if (building && building->get_type() == type && building->get_owner() == player->get_index()) {
                    unsigned int flag_index = building->get_flag_index();
                    if (flag_index != 0) {
                        Flag* flag = game->get_flag(flag_index);
                        if (flag) {
                            MapPos flag_pos = flag->get_position();
                            AILogger::log_debug("Building flag found at position " + std::to_string(flag_pos) + " for type " + std::to_string(type));
                            return flag_pos;
                        }
                    }
                }
            }
        }
    }
    
    AILogger::log_debug("Building flag search failed for type " + std::to_string(type));
    return 0;
}

bool ScriptedAgent::road_exists_between(MapPos flag1, MapPos flag2, const GameState& state, Game* game, Player* player) {
    AILogger::log_debug("Road existence check: " + std::to_string(flag1) + " <-> " + std::to_string(flag2));
    
    if (game != nullptr && flag1 != 0 && flag2 != 0 && flag1 != flag2) {
        // Get flags directly by position
        Flag* flag1_obj = game->get_flag_at_pos(flag1);
        Flag* flag2_obj = game->get_flag_at_pos(flag2);
        
        if (flag1_obj && flag2_obj) {
            // Check if flag1 has any paths that lead to flag2
            for (int dir = 0; dir < 6; dir++) {
                if (flag1_obj->has_path(static_cast<Direction>(dir))) {
                    Flag* connected_flag = flag1_obj->get_other_end_flag(static_cast<Direction>(dir));
                    if (connected_flag && connected_flag->get_position() == flag2) {
                        AILogger::log_debug("Road exists: direct connection found");
                        return true;
                    }
                }
            }
        }
    }
    
    AILogger::log_debug("Road existence check: no road found");
    return false;
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
    if (pos >= map.terrain_types.size()) {
        AILogger::log_debug("Castle pos " + std::to_string(pos) + " FAIL: exceeds map bounds (max: " + 
                           std::to_string(map.terrain_types.size()) + ")");
        return false;
    }
    
    // Check if position already has building
    if (map.has_building[pos]) {
        AILogger::log_debug("Castle pos " + std::to_string(pos) + " FAIL: already has building");
        return false;
    }
    
    // Check if position has flag
    if (map.has_flag[pos]) {
        AILogger::log_debug("Castle pos " + std::to_string(pos) + " FAIL: already has flag");
        return false;
    }
    
    // Check ownership - for castle, position should be unowned
    if (map.ownership[pos] != 255) {  // 255 typically means unowned
        AILogger::log_debug("Castle pos " + std::to_string(pos) + " FAIL: owned by player " + 
                           std::to_string(map.ownership[pos]));
        return false;
    }
    
    // Check terrain type (simplified check)
    uint8_t terrain = map.terrain_types[pos];
    if (terrain >= 8) {  // Avoid water and very steep terrain
        AILogger::log_debug("Castle pos " + std::to_string(pos) + " FAIL: unsuitable terrain type " + 
                           std::to_string(terrain));
        return false;
    }
    
    // Check that we're not too close to water or other obstacles
    int x = pos % map.width;
    int y = pos / map.width;
    
    // Basic check of surrounding positions (simplified spiral check)
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            
            int check_x = x + dx;
            int check_y = y + dy;
            
            if (check_x < 0 || check_x >= map.width || check_y < 0 || check_y >= map.height) {
                continue;  // Out of bounds, skip
            }
            
            MapPos check_pos = check_y * map.width + check_x;
            if (check_pos >= map.terrain_types.size()) continue;
            
            // Check if adjacent position is already owned
            if (map.ownership[check_pos] != 255) {
                AILogger::log_debug("Castle pos " + std::to_string(pos) + " FAIL: adjacent position " + 
                                   std::to_string(check_pos) + " is owned");
                return false;
            }
            
            // Check if adjacent position has building
            if (map.has_building[check_pos]) {
                AILogger::log_debug("Castle pos " + std::to_string(pos) + " FAIL: adjacent position " + 
                                   std::to_string(check_pos) + " has building");
                return false;
            }
        }
    }
    
    AILogger::log_debug("Castle pos " + std::to_string(pos) + " PASS: all checks passed, terrain=" + 
                       std::to_string(terrain));
    return true;
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
    
    // Use safe dimensions
    size_t actual_map_size = map.terrain_types.size();
    if (actual_map_size == 0) return 0;
    
    int safe_width = static_cast<int>(std::sqrt(actual_map_size));
    
    int x_center = pos % safe_width;
    int y_center = pos / safe_width;
    
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx * dx + dy * dy > radius * radius) continue;
            
            int x = x_center + dx;
            int y = y_center + dy;
            
            if (x < 0 || x >= safe_width || y < 0 || y >= safe_width) continue;
            
            MapPos check_pos = y * safe_width + x;
            
            // Bounds check
            if (check_pos >= actual_map_size) continue;
            
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
                // Keep the castle_position that was set when we built the castle
                // Don't reset it to 0 as the building list extraction is not yet implemented
                if (castle_position == 0) {
                    // Fallback: try to find castle position from building list
                    for (size_t i = 0; i < state.self.building_positions.size(); i++) {
                        if (state.self.building_types[i] == Building::TypeCastle) {
                            castle_position = state.self.building_positions[i];
                            break;
                        }
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
            // Transition to producing after some time or when we've placed some flags
            // Simple heuristic: if we have spent some time in this state, move on
            if (state.game_tick > 300) {  // After some time building flags
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