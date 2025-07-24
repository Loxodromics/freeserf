#include "agent-integration.h"
#include "player-agent-extensions.h"
#include "agent-factory.h"
#include "ai-logger.h"
#include "../game-manager.h"
#include "../building.h"
#include "../pathfinder.h"

void AgentIntegration::attach_agent(Player* player, std::unique_ptr<Agent> agent) {
    PlayerAgentExtensions::set_agent(player, std::move(agent));
}

void AgentIntegration::detach_agent(Player* player) {
    PlayerAgentExtensions::set_agent(player, nullptr);
}

Agent* AgentIntegration::get_agent(const Player* player) {
    return PlayerAgentExtensions::get_agent(const_cast<Player*>(player));
}

bool AgentIntegration::has_agent(const Player* player) {
    return PlayerAgentExtensions::has_agent(player);
}

GameState AgentIntegration::capture_game_state(const Game* game, const Player* player) {
    GameState state = {};
    
    // Game timing
    state.game_tick = game->get_tick();
    state.game_speed = 1;  // TODO: Get actual game speed from game settings
    
    // Extract self (this AI player's) state
    extract_player_state(state.self, player, game);
    
    // Extract opponent states
    // TODO: Get total player count from game and iterate through other players
    state.opponents.clear();
    // For now, just extract self state - will implement opponent extraction later
    
    // Extract map information
    extract_map_info(state.map, game);
    
    // Extract global information
    extract_global_info(state.global, game);
    
    return state;
}

void AgentIntegration::extract_player_state(GameState::PlayerState& player_state, 
                                           const Player* player, const Game* game) {
    // Basic player information
    player_state.player_index = player->get_index();
    player_state.is_human = !player->is_ai();
    player_state.has_castle = player->has_castle();
    
    // Extract resources using public API (cast away const for stats access)
    ResourceMap resources = const_cast<Player*>(player)->get_stats_resources();
    for (int i = 0; i < 26; ++i) {
        Resource::Type res_type = static_cast<Resource::Type>(i);
        auto it = resources.find(res_type);
        player_state.resource_counts[i] = (it != resources.end()) ? it->second : 0;
        player_state.resource_priorities[i] = player->get_inventory_prio(i);
    }
    
    // Extract building counts (24 building types) - include both completed and under construction
    for (int i = 0; i < 24; ++i) {
        int completed = player->get_completed_building_count(i);
        int incomplete = player->get_incomplete_building_count(i);
        player_state.building_counts[i] = completed + incomplete;  // Total buildings (completed + under construction)
        
        // Debug logging for key building types
        if (i == Building::TypeForester || i == Building::TypeLumberjack || i == Building::TypeCastle) {
            AILogger::log_debug("Building type " + std::to_string(i) + ": completed=" + std::to_string(completed) + 
                               ", incomplete=" + std::to_string(incomplete) + ", total=" + std::to_string(player_state.building_counts[i]));
        }
    }
    
    // Military information
    player_state.castle_knights = player->get_castle_knights();
    player_state.knight_morale = player->get_knight_morale();
    
    // Extract knight counts by level (5 knight types: Knight0-Knight4)
    for (int i = 0; i < 5; ++i) {
        player_state.knight_counts[i] = player->get_serf_count(Serf::TypeKnight0 + i);
    }
    
    // Territory and economic stats
    player_state.territory_size = player->get_land_area();
    player_state.total_serfs = 0;
    player_state.idle_serfs = 0;
    
    // Sum up all serf types for total count
    for (int i = 0; i < 27; ++i) {  // 27 serf types total
        player_state.total_serfs += player->get_serf_count(i);
    }
    player_state.idle_serfs = player->get_serf_count(Serf::TypeGeneric);
    
    player_state.economic_score = static_cast<float>(player->get_building_score());
    player_state.military_score = static_cast<float>(player->get_military_score());
    
    // Extract building positions and types
    // TODO: Implement building position extraction from game building collections
    player_state.building_positions.clear();
    player_state.building_types.clear();
    
    // Extract owned territory positions  
    // TODO: Implement territory position extraction from map ownership data
    player_state.owned_territory.clear();
}

void AgentIntegration::extract_map_info(GameState::MapInfo& map_info, const Game* game) {
    // Cast away const to access map - read-only access for AI
    PMap map = const_cast<Game*>(game)->get_map();
    
    // Basic map dimensions
    map_info.width = map->get_cols();
    map_info.height = map->get_rows();
    
    // Debug: Log actual map dimensions and size
    AILogger::log_debug("Map extraction: cols=" + std::to_string(map_info.width) + 
                       ", rows=" + std::to_string(map_info.height));
    AILogger::log_debug("Map size (get_size): " + std::to_string(map->get_size()));
    AILogger::log_debug("Calculated size (width*height): " + std::to_string(map_info.width * map_info.height));
    
    size_t map_size = map_info.width * map_info.height;
    
    // Initialize map data vectors
    map_info.terrain_types.resize(map_size);
    map_info.elevation.resize(map_size);
    map_info.ownership.resize(map_size);
    map_info.has_building.resize(map_size);
    map_info.has_flag.resize(map_size);
    map_info.has_road.resize(map_size);
    
    // Extract terrain and ownership data for each map position
    for (MapPos pos = 0; pos < map_size; ++pos) {
        // Terrain type (simplified - using terrain up)
        map_info.terrain_types[pos] = static_cast<uint8_t>(map->type_up(pos));
        
        // Elevation
        map_info.elevation[pos] = static_cast<uint8_t>(map->get_height(pos));
        
        // Ownership (player index, or 0xFF if unowned)
        if (map->has_owner(pos)) {
            map_info.ownership[pos] = static_cast<uint8_t>(map->get_owner(pos));
        } else {
            map_info.ownership[pos] = 0xFF;  // Unowned
        }
        
        // Objects on map
        map_info.has_building[pos] = map->has_building(pos);
        map_info.has_flag[pos] = map->has_flag(pos);
        // TODO: Check for roads - need to examine map object system
        map_info.has_road[pos] = false;  // Placeholder
    }
    
    // Extract resource deposits
    // TODO: Implement resource deposit extraction by scanning map for mineral types
    map_info.stone_deposits.clear();
    map_info.coal_deposits.clear();
    map_info.iron_deposits.clear();
    map_info.gold_deposits.clear();
    map_info.water_tiles.clear();
}

void AgentIntegration::extract_global_info(GameState::GlobalInfo& global_info, const Game* game) {
    // TODO: Get actual player count from game
    global_info.total_players = 4;  // Placeholder - common game size
    global_info.active_players = 4; // Placeholder
    global_info.game_ended = false; // TODO: Check game end conditions
    global_info.winning_player = -1; // Game ongoing
    
    // Performance budgeting (2-3ms per player per tick target)
    global_info.ai_time_budget_ms = 2.5f;
    global_info.last_execution_time_ms = 0.0f; // TODO: Track actual execution time
}

void AgentIntegration::update_game_state(GameState& state, const Game* game, const Player* player) {
    // Update only dynamic parts of game state for efficiency
    state.game_tick = game->get_tick();
    
    // Update self player state
    extract_player_state(state.self, player, game);
    
    // TODO: Update opponent states efficiently
    // TODO: Update only changed map areas if needed for performance
}

// ActionValidator implementation
AgentIntegration::ActionValidationResult AgentIntegration::ActionValidator::validate_action(
    const AIAction& action, const Game* game, const Player* player) {
    
    switch (action.type) {
        case AIActionType::BUILD_CASTLE:
            return validate_build_castle(action.primary_position, game, player);
            
        case AIActionType::BUILD_FLAG:
            return validate_build_flag(action.primary_position, game, player);
            
        case AIActionType::BUILD_ROAD:
            return validate_build_road(action.primary_position, action.secondary_position, game, player);
            
        case AIActionType::BUILD_LUMBERJACK:
            return validate_build_building(action.primary_position, Building::TypeLumberjack, game, player);
            
        case AIActionType::BUILD_FORESTER:
            return validate_build_building(action.primary_position, Building::TypeForester, game, player);
            
        // Phase 0.8.1 New Building Validations
        case AIActionType::BUILD_FISHER:
            return validate_build_building(action.primary_position, Building::TypeFisher, game, player);
        case AIActionType::BUILD_BOATBUILDER:
            return validate_build_building(action.primary_position, Building::TypeBoatbuilder, game, player);
        case AIActionType::BUILD_STONECUTTER:
            return validate_build_building(action.primary_position, Building::TypeStonecutter, game, player);
        case AIActionType::BUILD_STONE_MINE:
            return validate_build_building(action.primary_position, Building::TypeStoneMine, game, player);
        case AIActionType::BUILD_COAL_MINE:
            return validate_build_building(action.primary_position, Building::TypeCoalMine, game, player);
        case AIActionType::BUILD_IRON_MINE:
            return validate_build_building(action.primary_position, Building::TypeIronMine, game, player);
        case AIActionType::BUILD_GOLD_MINE:
            return validate_build_building(action.primary_position, Building::TypeGoldMine, game, player);
        case AIActionType::BUILD_STOCK:
            return validate_build_building(action.primary_position, Building::TypeStock, game, player);
        case AIActionType::BUILD_HUT:
            return validate_build_building(action.primary_position, Building::TypeHut, game, player);
        case AIActionType::BUILD_FARM:
            return validate_build_building(action.primary_position, Building::TypeFarm, game, player);
        case AIActionType::BUILD_BUTCHER:
            return validate_build_building(action.primary_position, Building::TypeButcher, game, player);
        case AIActionType::BUILD_PIG_FARM:
            return validate_build_building(action.primary_position, Building::TypePigFarm, game, player);
        case AIActionType::BUILD_MILL:
            return validate_build_building(action.primary_position, Building::TypeMill, game, player);
        case AIActionType::BUILD_BAKER:
            return validate_build_building(action.primary_position, Building::TypeBaker, game, player);
        case AIActionType::BUILD_SAWMILL:
            return validate_build_building(action.primary_position, Building::TypeSawmill, game, player);
        case AIActionType::BUILD_STEEL_SMELTER:
            return validate_build_building(action.primary_position, Building::TypeSteelSmelter, game, player);
        case AIActionType::BUILD_TOOL_MAKER:
            return validate_build_building(action.primary_position, Building::TypeToolMaker, game, player);
        case AIActionType::BUILD_WEAPON_SMITH:
            return validate_build_building(action.primary_position, Building::TypeWeaponSmith, game, player);
        case AIActionType::BUILD_TOWER:
            return validate_build_building(action.primary_position, Building::TypeTower, game, player);
        case AIActionType::BUILD_FORTRESS:
            return validate_build_building(action.primary_position, Building::TypeFortress, game, player);
        case AIActionType::BUILD_GOLD_SMELTER:
            return validate_build_building(action.primary_position, Building::TypeGoldSmelter, game, player);
            
        // Demolition action validations
        case AIActionType::DEMOLISH_BUILDING:
            return validate_demolish_building(action.primary_position, game, player);
        case AIActionType::DEMOLISH_FLAG:
            return validate_demolish_flag(action.primary_position, game, player);
        case AIActionType::DEMOLISH_ROAD:
            return validate_demolish_road(action.primary_position, game, player);
            
        case AIActionType::NO_ACTION:
        case AIActionType::WAIT:
            return {true, "No action or wait - always valid", ActionError::SUCCESS, 1.0f};
            
        default:
            return {false, "Unknown action type", ActionError::UNKNOWN_ERROR, 0.0f};
    }
}

AgentIntegration::ActionValidationResult AgentIntegration::ActionValidator::validate_build_castle(
    MapPos pos, const Game* game, const Player* player) {
    
    Game* mutable_game = const_cast<Game*>(game);
    Player* mutable_player = const_cast<Player*>(player);
    
    // Check if player already has a castle
    if (player->has_castle()) {
        return {false, "Player already has a castle", ActionError::INVALID_POSITION, 0.0f};
    }
    
    // First, try the AI's suggested position with authoritative validation
    if (mutable_game->can_build_castle(pos, player)) {
        AILogger::log_debug("Castle validation: AI position " + std::to_string(pos) + " is valid");
        return {true, "Castle placement valid", ActionError::SUCCESS, 1.0f};
    }
    
    // AI's position failed - use smart position finding to find a valid alternative
    AILogger::log_debug("Castle validation: AI position " + std::to_string(pos) + " invalid, searching for alternative...");
    
    MapPos alternative_pos = find_valid_castle_position_nearby(pos, mutable_game, mutable_player);
    
    if (alternative_pos != 0) {
        AILogger::log_debug("Castle validation: Found alternative position " + std::to_string(alternative_pos));
        return {true, "Castle placement valid (corrected position)", ActionError::SUCCESS, 1.0f, alternative_pos};
    }
    
    // No valid position found
    AILogger::log_debug("Castle validation: No valid castle position found");
    return {false, "No valid castle position available", ActionError::INVALID_POSITION, 0.0f};
}

// Helper method to find valid castle position near the AI's suggestion
MapPos AgentIntegration::ActionValidator::find_valid_castle_position_nearby(MapPos suggested_pos, Game* game, Player* player) {
    auto map = game->get_map();
    int map_cols = map->get_cols();
    int map_rows = map->get_rows();
    
    // Convert suggested position to coordinates for center-outward search
    int center_x = suggested_pos % map_cols;
    int center_y = suggested_pos / map_cols;
    
    int positions_tested = 0;
    const int max_positions = 300; // Increase search limit for better coverage
    
    // Search in expanding radius around the AI's suggested position
    for (int radius = 1; radius <= 30 && positions_tested < max_positions; radius += 2) {
        // Test positions in a circle around the suggested position
        for (int angle = 0; angle < 360 && positions_tested < max_positions; angle += 30) {
            double rad = angle * 3.14159 / 180.0;
            int x = center_x + static_cast<int>(radius * std::cos(rad));
            int y = center_y + static_cast<int>(radius * std::sin(rad));
            
            // Bounds check
            if (x < 5 || x >= map_cols - 5 || y < 5 || y >= map_rows - 5) {
                continue;
            }
            
            MapPos test_pos = y * map_cols + x;
            positions_tested++;
            
            // Use authoritative game validation
            if (game->can_build_castle(test_pos, player)) {
                AILogger::log_debug("Found valid castle position: " + std::to_string(test_pos) + 
                                   " (" + std::to_string(x) + "," + std::to_string(y) + 
                                   ") radius=" + std::to_string(radius) + 
                                   " tested=" + std::to_string(positions_tested));
                return test_pos;
            }
        }
    }
    
    AILogger::log_debug("No valid castle position found after testing " + std::to_string(positions_tested) + " positions");
    return 0; // No valid position found
}

// Helper method to find valid building position near the AI's suggestion
MapPos AgentIntegration::ActionValidator::find_valid_building_position_nearby(MapPos suggested_pos, Building::Type type, Game* game, Player* player) {
    auto map = game->get_map();
    int map_cols = map->get_cols();
    int map_rows = map->get_rows();
    
    // Convert suggested position to coordinates for center-outward search
    int center_x = suggested_pos % map_cols;
    int center_y = suggested_pos / map_cols;
    
    int positions_tested = 0;
    const int max_positions = 100; // Limit search to stay within performance budget
    
    // Search in expanding radius around the AI's suggested position
    for (int radius = 1; radius <= 15 && positions_tested < max_positions; radius += 2) {
        // Test positions in a circle around the suggested position
        for (int angle = 0; angle < 360 && positions_tested < max_positions; angle += 45) {
            double rad = angle * 3.14159 / 180.0;
            int x = center_x + static_cast<int>(radius * std::cos(rad));
            int y = center_y + static_cast<int>(radius * std::sin(rad));
            
            // Bounds check
            if (x < 2 || x >= map_cols - 2 || y < 2 || y >= map_rows - 2) {
                continue;
            }
            
            MapPos test_pos = y * map_cols + x;
            positions_tested++;
            
            // Use authoritative game validation
            if (game->can_build_building(test_pos, type, player)) {
                AILogger::log_debug("Found valid building position: " + std::to_string(test_pos) + 
                                   " (type " + std::to_string(type) + ") " +
                                   " (:" + std::to_string(x) + "," + std::to_string(y) + 
                                   ") radius=" + std::to_string(radius) + 
                                   " tested=" + std::to_string(positions_tested));
                return test_pos;
            }
        }
    }
    
    AILogger::log_debug("No valid building position found for type " + std::to_string(type) + 
                       " after testing " + std::to_string(positions_tested) + " positions");
    return 0; // No valid position found
}

AgentIntegration::ActionValidationResult AgentIntegration::ActionValidator::validate_build_flag(
    MapPos pos, const Game* game, const Player* player) {
    
    // Use existing game validation method
    if (!const_cast<Game*>(game)->can_build_flag(pos, player)) {
        return {false, "Cannot build flag at position", ActionError::INVALID_POSITION, 1.0f};
    }
    
    return {true, "Flag placement valid", ActionError::SUCCESS, 1.0f};
}

AgentIntegration::ActionValidationResult AgentIntegration::ActionValidator::validate_build_road(
    MapPos from, MapPos to, const Game* game, const Player* player) {
    
    // Create a simple road from start to end for validation
    Road road;
    road.start(from);
    
    // For now, do basic validation - full pathfinding will be done in execution
    if (from == to) {
        return {false, "Road start and end positions are the same", ActionError::INVALID_ROAD_PATH, 1.0f};
    }
    
    MapPos dest;
    bool water;
    int result = const_cast<Game*>(game)->can_build_road(road, player, &dest, &water);
    
    if (result < 0) {
        return {false, "Cannot build road at specified location", ActionError::INVALID_ROAD_PATH, 1.0f};
    }
    
    return {true, "Road path valid", ActionError::SUCCESS, 1.0f};
}

AgentIntegration::ActionValidationResult AgentIntegration::ActionValidator::validate_build_building(
    MapPos pos, Building::Type type, const Game* game, const Player* player) {
    
    Game* mutable_game = const_cast<Game*>(game);
    Player* mutable_player = const_cast<Player*>(player);
    
    // First, try the AI's suggested position with authoritative validation
    if (mutable_game->can_build_building(pos, type, player)) {
        AILogger::log_debug("Building validation: AI position " + std::to_string(pos) + " is valid for building type " + std::to_string(type));
        return {true, "Building placement valid", ActionError::SUCCESS, 1.0f};
    }
    
    // AI's position failed - use smart position finding to find a valid alternative
    AILogger::log_debug("Building validation: AI position " + std::to_string(pos) + " invalid for building type " + std::to_string(type) + ", searching for alternative...");
    
    MapPos alternative_pos = find_valid_building_position_nearby(pos, type, mutable_game, mutable_player);
    
    if (alternative_pos != 0) {
        AILogger::log_debug("Building validation: Found alternative position " + std::to_string(alternative_pos) + " for building type " + std::to_string(type));
        return {true, "Building placement valid (corrected position)", ActionError::SUCCESS, 1.0f, alternative_pos};
    }
    
    // No valid position found
    AILogger::log_debug("Building validation: No valid building position found for type " + std::to_string(type));
    return {false, "No valid building position available", ActionError::INVALID_POSITION, 0.0f};
}

// ActionExecutor implementation
std::vector<AgentIntegration::ActionResult> AgentIntegration::ActionExecutor::execute_actions(
    const std::vector<AIAction>& actions, Game* game, Player* player) {
    
    std::vector<ActionResult> results;
    
    for (const auto& action : actions) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        ActionResult result;
        
        switch (action.type) {
            case AIActionType::BUILD_CASTLE:
                result = execute_build_castle(action, game, player);
                break;
                
            case AIActionType::BUILD_FLAG:
                result = execute_build_flag(action, game, player);
                break;
                
            case AIActionType::BUILD_ROAD:
                result = execute_build_road(action, game, player);
                break;
                
            case AIActionType::BUILD_LUMBERJACK:
                result = execute_build_lumberjack(action, game, player);
                break;
                
            case AIActionType::BUILD_FORESTER:
                result = execute_build_forester(action, game, player);
                break;
                
            // Phase 0.8.1 New Building Executions
            case AIActionType::BUILD_FISHER:
                result = execute_build_building_generic(action, Building::TypeFisher, game, player);
                break;
            case AIActionType::BUILD_BOATBUILDER:
                result = execute_build_building_generic(action, Building::TypeBoatbuilder, game, player);
                break;
            case AIActionType::BUILD_STONECUTTER:
                result = execute_build_building_generic(action, Building::TypeStonecutter, game, player);
                break;
            case AIActionType::BUILD_STONE_MINE:
                result = execute_build_building_generic(action, Building::TypeStoneMine, game, player);
                break;
            case AIActionType::BUILD_COAL_MINE:
                result = execute_build_building_generic(action, Building::TypeCoalMine, game, player);
                break;
            case AIActionType::BUILD_IRON_MINE:
                result = execute_build_building_generic(action, Building::TypeIronMine, game, player);
                break;
            case AIActionType::BUILD_GOLD_MINE:
                result = execute_build_building_generic(action, Building::TypeGoldMine, game, player);
                break;
            case AIActionType::BUILD_STOCK:
                result = execute_build_building_generic(action, Building::TypeStock, game, player);
                break;
            case AIActionType::BUILD_HUT:
                result = execute_build_building_generic(action, Building::TypeHut, game, player);
                break;
            case AIActionType::BUILD_FARM:
                result = execute_build_building_generic(action, Building::TypeFarm, game, player);
                break;
            case AIActionType::BUILD_BUTCHER:
                result = execute_build_building_generic(action, Building::TypeButcher, game, player);
                break;
            case AIActionType::BUILD_PIG_FARM:
                result = execute_build_building_generic(action, Building::TypePigFarm, game, player);
                break;
            case AIActionType::BUILD_MILL:
                result = execute_build_building_generic(action, Building::TypeMill, game, player);
                break;
            case AIActionType::BUILD_BAKER:
                result = execute_build_building_generic(action, Building::TypeBaker, game, player);
                break;
            case AIActionType::BUILD_SAWMILL:
                result = execute_build_building_generic(action, Building::TypeSawmill, game, player);
                break;
            case AIActionType::BUILD_STEEL_SMELTER:
                result = execute_build_building_generic(action, Building::TypeSteelSmelter, game, player);
                break;
            case AIActionType::BUILD_TOOL_MAKER:
                result = execute_build_building_generic(action, Building::TypeToolMaker, game, player);
                break;
            case AIActionType::BUILD_WEAPON_SMITH:
                result = execute_build_building_generic(action, Building::TypeWeaponSmith, game, player);
                break;
            case AIActionType::BUILD_TOWER:
                result = execute_build_building_generic(action, Building::TypeTower, game, player);
                break;
            case AIActionType::BUILD_FORTRESS:
                result = execute_build_building_generic(action, Building::TypeFortress, game, player);
                break;
            case AIActionType::BUILD_GOLD_SMELTER:
                result = execute_build_building_generic(action, Building::TypeGoldSmelter, game, player);
                break;
                
            // Demolition action executions
            case AIActionType::DEMOLISH_BUILDING:
                result = execute_demolish_building(action, game, player);
                break;
            case AIActionType::DEMOLISH_FLAG:
                result = execute_demolish_flag(action, game, player);
                break;
            case AIActionType::DEMOLISH_ROAD:
                result = execute_demolish_road(action, game, player);
                break;
                
            case AIActionType::NO_ACTION:
            case AIActionType::WAIT:
                {
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
                    result = create_success_result("No action or wait completed", 0.0f, duration);
                }
                break;
                
            default:
                {
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
                    result = create_failure_result("Unknown action type", ActionError::UNKNOWN_ERROR, duration);
                }
                break;
        }
        
        results.push_back(result);
    }
    
    return results;
}

AgentIntegration::ActionResult AgentIntegration::ActionExecutor::execute_build_castle(
    const AIAction& action, Game* game, Player* player) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Validate first
    auto validation = ActionValidator::validate_build_castle(action.primary_position, game, player);
    if (!validation.is_valid) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result(validation.failure_reason, validation.error_code, duration);
    }
    
    // Execute using existing game method
    if (game->build_castle(action.primary_position, player)) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_success_result("Castle built successfully", 10.0f, duration);
    } else {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result("Failed to build castle", ActionError::GAME_ENGINE_ERROR, duration);
    }
}

AgentIntegration::ActionResult AgentIntegration::ActionExecutor::execute_build_flag(
    const AIAction& action, Game* game, Player* player) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Validate first
    auto validation = ActionValidator::validate_build_flag(action.primary_position, game, player);
    if (!validation.is_valid) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result(validation.failure_reason, validation.error_code, duration);
    }
    
    // Execute using existing game method
    if (game->build_flag(action.primary_position, player)) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_success_result("Flag built successfully", 1.0f, duration);
    } else {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result("Failed to build flag", ActionError::GAME_ENGINE_ERROR, duration);
    }
}

AgentIntegration::ActionResult AgentIntegration::ActionExecutor::execute_build_lumberjack(
    const AIAction& action, Game* game, Player* player) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Validate first
    auto validation = ActionValidator::validate_build_building(action.primary_position, Building::TypeLumberjack, game, player);
    if (!validation.is_valid) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result(validation.failure_reason, validation.error_code, duration);
    }
    
    // Execute using existing game method
    if (game->build_building(action.primary_position, Building::TypeLumberjack, player)) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_success_result("Lumberjack built successfully", 5.0f, duration);
    } else {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result("Failed to build lumberjack", ActionError::GAME_ENGINE_ERROR, duration);
    }
}

AgentIntegration::ActionResult AgentIntegration::ActionExecutor::execute_build_forester(
    const AIAction& action, Game* game, Player* player) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Validate first
    auto validation = ActionValidator::validate_build_building(action.primary_position, Building::TypeForester, game, player);
    if (!validation.is_valid) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result(validation.failure_reason, validation.error_code, duration);
    }
    
    // Execute using existing game method
    if (game->build_building(action.primary_position, Building::TypeForester, player)) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_success_result("Forester built successfully", 5.0f, duration);
    } else {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result("Failed to build forester", ActionError::GAME_ENGINE_ERROR, duration);
    }
}

AgentIntegration::ActionResult AgentIntegration::ActionExecutor::execute_build_road(
    const AIAction& action, Game* game, Player* player) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    MapPos from = action.primary_position;
    MapPos to = action.secondary_position;
    
    // Basic validation
    if (from == to) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result("Road start and end positions are the same", ActionError::INVALID_ROAD_PATH, duration);
    }
    
    // Use pathfinder to create road
    PMap map = game->get_map();
    Road road = pathfinder_map(map.get(), from, to);
    
    if (!road.is_valid()) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result("No valid path found for road", ActionError::INVALID_ROAD_PATH, duration);
    }
    
    // Validate the road can be built
    MapPos dest;
    bool water;
    int result = game->can_build_road(road, player, &dest, &water);
    
    if (result < 0) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result("Cannot build road at specified location", ActionError::INVALID_ROAD_PATH, duration);
    }
    
    // Execute road building
    if (game->build_road(road, player)) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_success_result("Road built successfully", 3.0f, duration);
    } else {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result("Failed to build road", ActionError::GAME_ENGINE_ERROR, duration);
    }
}

AgentIntegration::ActionResult AgentIntegration::ActionExecutor::create_success_result(
    const std::string& message, float reward, std::chrono::microseconds exec_time) {
    
    return {true, reward, message, ActionError::SUCCESS, exec_time};
}

AgentIntegration::ActionResult AgentIntegration::ActionExecutor::create_failure_result(
    const std::string& message, ActionError error, std::chrono::microseconds exec_time) {
    
    return {false, 0.0f, message, error, exec_time};
}

std::vector<AgentIntegration::ActionResult> AgentIntegration::execute_actions(
    const std::vector<AIAction>& actions,
    Game* game, 
    Player* player) {
    
    // Delegate to ActionExecutor
    return ActionExecutor::execute_actions(actions, game, player);
}

AgentIntegration::ActionResult AgentIntegration::ActionExecutor::execute_build_building_generic(
    const AIAction& action, Building::Type building_type, Game* game, Player* player) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Validate first
    auto validation = ActionValidator::validate_build_building(action.primary_position, building_type, game, player);
    if (!validation.is_valid) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result(validation.failure_reason, validation.error_code, duration);
    }
    
    // Execute using existing game method
    if (game->build_building(action.primary_position, building_type, player)) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        float reward = calculate_building_reward(building_type);
        std::string success_message = "Building " + std::to_string(static_cast<int>(building_type)) + " built successfully";
        return create_success_result(success_message, reward, duration);
    } else {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::string failure_message = "Failed to build building " + std::to_string(static_cast<int>(building_type));
        return create_failure_result(failure_message, ActionError::GAME_ENGINE_ERROR, duration);
    }
}

float AgentIntegration::ActionExecutor::calculate_building_reward(Building::Type type) {
    switch (type) {
        // Basic production
        case Building::TypeFisher:
        case Building::TypeFarm:
        case Building::TypeButcher:
        case Building::TypePigFarm:
        case Building::TypeMill:
        case Building::TypeBaker:
            return 5.0f;
            
        // Resource extraction
        case Building::TypeLumberjack:
        case Building::TypeForester:
        case Building::TypeStonecutter:
            return 6.0f;
            
        // Advanced manufacturing  
        case Building::TypeBoatbuilder:
        case Building::TypeSawmill:
        case Building::TypeSteelSmelter:
        case Building::TypeToolMaker:
        case Building::TypeWeaponSmith:
        case Building::TypeGoldSmelter:
            return 7.0f;
            
        // Mining (valuable)
        case Building::TypeStoneMine:
        case Building::TypeCoalMine:
        case Building::TypeIronMine:
        case Building::TypeGoldMine:
            return 8.0f;
            
        // Military (strategic)
        case Building::TypeHut:
            return 6.0f;
        case Building::TypeTower:
            return 8.0f;
        case Building::TypeFortress:
            return 10.0f;
            
        // Storage (utility)
        case Building::TypeStock:
            return 4.0f;
            
        // Castle (special)
        case Building::TypeCastle:
            return 10.0f;
            
        default:
            return 5.0f;
    }
}

AgentIntegration::PerformanceMetrics AgentIntegration::get_performance_metrics(const Player* player) {
    PerformanceMetrics metrics = {};
    
    // TODO: Implement performance tracking in Phase 0.3
    metrics.last_execution_time_ms = 0.0f;
    metrics.average_execution_time_ms = 0.0f;
    metrics.successful_actions = 0;
    metrics.failed_actions = 0;
    metrics.budget_exceeded = false;
    
    return metrics;
}

void AgentIntegration::reset_performance_metrics(Player* player) {
    // TODO: Implement performance tracking reset in Phase 0.3
}

void AgentIntegration::agent_player_update(Player* player, Game* game, uint16_t tick_delta) {
    PlayerAgentExtensions::update_agent_player(player, game, tick_delta);
}

void AgentIntegration::agent_game_started(const Game* game) {
    // TODO: Implement game lifecycle management in Phase 0.4
}

void AgentIntegration::agent_game_ended(const Game* game, bool victory) {
    // TODO: Implement game lifecycle management in Phase 0.4
}

// Demolition validation methods
AgentIntegration::ActionValidationResult AgentIntegration::ActionValidator::validate_demolish_building(
    MapPos pos, const Game* game, const Player* player) {
    
    // Check if there's a building at this position that belongs to the player
    PMap map = const_cast<Game*>(game)->get_map();
    if (!map->has_building(pos)) {
        return {false, "No building at position", ActionError::INVALID_POSITION, 0.0f};
    }
    
    // Get building and check ownership
    Building* building = const_cast<Game*>(game)->get_building_at_pos(pos);
    if (!building) {
        return {false, "No building at position", ActionError::INVALID_POSITION, 0.0f};
    }
    
    if (building->get_owner() != player->get_index()) {
        return {false, "Building not owned by player", ActionError::INVALID_POSITION, 0.0f};
    }
    
    // Check if building is in a state that can be demolished (not burning, etc.)
    if (building->is_burning()) {
        return {false, "Cannot demolish burning building", ActionError::INVALID_POSITION, 0.0f};
    }
    
    return {true, "Building can be demolished", ActionError::SUCCESS, 1.0f};
}

AgentIntegration::ActionValidationResult AgentIntegration::ActionValidator::validate_demolish_flag(
    MapPos pos, const Game* game, const Player* player) {
    
    // Use existing game validation method
    bool can_demolish = game->can_demolish_flag(pos, player);
    
    if (can_demolish) {
        return {true, "Flag can be demolished", ActionError::SUCCESS, 1.0f};
    } else {
        return {false, "Cannot demolish flag at position", ActionError::INVALID_POSITION, 0.0f};
    }
}

AgentIntegration::ActionValidationResult AgentIntegration::ActionValidator::validate_demolish_road(
    MapPos pos, const Game* game, const Player* player) {
    
    // Use existing game validation method
    bool can_demolish = game->can_demolish_road(pos, player);
    
    if (can_demolish) {
        return {true, "Road can be demolished", ActionError::SUCCESS, 1.0f};
    } else {
        return {false, "Cannot demolish road at position", ActionError::INVALID_POSITION, 0.0f};
    }
}

// Demolition execution methods
AgentIntegration::ActionResult AgentIntegration::ActionExecutor::execute_demolish_building(
    const AIAction& action, Game* game, Player* player) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Validate first
    auto validation = ActionValidator::validate_demolish_building(action.primary_position, game, player);
    if (!validation.is_valid) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result(validation.failure_reason, validation.error_code, duration);
    }
    
    // Execute demolition using existing game method
    bool success = game->demolish_building(action.primary_position, player);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto execution_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    if (success) {
        return create_success_result("Building demolished successfully", -2.0f, execution_time);
    } else {
        return create_failure_result("Building demolition failed", ActionError::GAME_ENGINE_ERROR, execution_time);
    }
}

AgentIntegration::ActionResult AgentIntegration::ActionExecutor::execute_demolish_flag(
    const AIAction& action, Game* game, Player* player) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Validate first
    auto validation = ActionValidator::validate_demolish_flag(action.primary_position, game, player);
    if (!validation.is_valid) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result(validation.failure_reason, validation.error_code, duration);
    }
    
    // Execute demolition using existing game method
    bool success = game->demolish_flag(action.primary_position, player);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto execution_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    if (success) {
        return create_success_result("Flag demolished successfully", -1.0f, execution_time);
    } else {
        return create_failure_result("Flag demolition failed", ActionError::GAME_ENGINE_ERROR, execution_time);
    }
}

AgentIntegration::ActionResult AgentIntegration::ActionExecutor::execute_demolish_road(
    const AIAction& action, Game* game, Player* player) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Validate first
    auto validation = ActionValidator::validate_demolish_road(action.primary_position, game, player);
    if (!validation.is_valid) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return create_failure_result(validation.failure_reason, validation.error_code, duration);
    }
    
    // Execute demolition using existing game method
    bool success = game->demolish_road(action.primary_position, player);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto execution_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    if (success) {
        return create_success_result("Road demolished successfully", -0.5f, execution_time);
    } else {
        return create_failure_result("Road demolition failed", ActionError::GAME_ENGINE_ERROR, execution_time);
    }
}

void AgentIntegration::setup_ai_players(int ai_count) {
    if (ai_count <= 0) return;
    
    // Get the current game from GameManager
    GameManager& game_manager = GameManager::get_instance();
    PGame game = game_manager.get_current_game();
    
    if (!game) {
        AILogger::log_error(-1, "Cannot setup AI players: no current game");
        return;
    }
    
    // Attach AI agents to the first ai_count players
    for (int i = 0; i < ai_count; ++i) {
        Player* player = game->get_player(i);
        if (player == nullptr) {
            AILogger::log_error(i, "Cannot setup AI: player not found");
            continue;
        }
        
        // Create a RandomAgent for testing complete build actions
        std::string agent_name = "RandomAgent_P" + std::to_string(i);
        auto agent = AgentFactory::create_random_agent(agent_name);
        
        if (agent) {
            attach_agent(player, std::move(agent));
            AILogger::log_agent_attached(i, agent_name);
        } else {
            AILogger::log_error(i, "Failed to create AI agent");
        }
    }
}