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
    
    // Extract building counts (24 building types)
    for (int i = 0; i < 24; ++i) {
        player_state.building_counts[i] = player->get_completed_building_count(i);
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
            
        case AIActionType::NO_ACTION:
        case AIActionType::WAIT:
            return {true, "No action or wait - always valid", ActionError::SUCCESS, 1.0f};
            
        default:
            return {false, "Unknown action type", ActionError::UNKNOWN_ERROR, 0.0f};
    }
}

AgentIntegration::ActionValidationResult AgentIntegration::ActionValidator::validate_build_castle(
    MapPos pos, const Game* game, const Player* player) {
    
    // Use existing game validation method
    if (!const_cast<Game*>(game)->can_build_castle(pos, player)) {
        return {false, "Cannot build castle at position", ActionError::INVALID_POSITION, 1.0f};
    }
    
    return {true, "Castle placement valid", ActionError::SUCCESS, 1.0f};
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
    
    // Use existing game validation method
    if (!const_cast<Game*>(game)->can_build_building(pos, type, player)) {
        return {false, "Cannot build building at position", ActionError::INVALID_POSITION, 1.0f};
    }
    
    return {true, "Building placement valid", ActionError::SUCCESS, 1.0f};
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
        
        // Create a ScriptedAgent
        std::string agent_name = "ScriptedAgent_P" + std::to_string(i);
        auto agent = AgentFactory::create_scripted_agent(5, 0, agent_name);
        
        if (agent) {
            attach_agent(player, std::move(agent));
            AILogger::log_agent_attached(i, agent_name);
        } else {
            AILogger::log_error(i, "Failed to create AI agent");
        }
    }
}