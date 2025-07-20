#include "agent-integration.h"
#include "player-agent-extensions.h"
#include "agent-factory.h"
#include "ai-logger.h"
#include "../game-manager.h"

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

std::vector<AgentIntegration::ActionResult> AgentIntegration::execute_actions(
    const std::vector<AIAction>& actions,
    Game* game, 
    Player* player) {
    
    std::vector<ActionResult> results;
    
    // TODO: Implement action execution in Phase 0.3
    // For now, return failed results for all actions
    for (const auto& action : actions) {
        ActionResult result;
        result.success = false;
        result.reward = 0.0f;
        result.failure_reason = "Action execution not yet implemented";
        result.error_code = -1;
        results.push_back(result);
    }
    
    return results;
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
        
        // Create a MockAgent for testing purposes
        std::string agent_name = "MockAgent_P" + std::to_string(i);
        auto agent = AgentFactory::create_scripted_agent(5, 0, agent_name);
        
        if (agent) {
            attach_agent(player, std::move(agent));
            AILogger::log_agent_attached(i, agent_name);
        } else {
            AILogger::log_error(i, "Failed to create AI agent");
        }
    }
}