#pragma once

#include <vector>
#include <array>
#include "../map.h"
#include "../resource.h"
#include "../building.h"

/**
 * Complete game state information for AI agents
 * This is a read-only snapshot of the current game state
 */
struct GameState {
    // Game timing
    uint32_t game_tick;
    uint32_t game_speed;
    
    // Player information
    struct PlayerState {
        int player_index;
        bool is_human;
        bool has_castle;
        
        // Resources (26 types)
        std::array<int, 26> resource_counts;
        std::array<int, 26> resource_priorities;
        
        // Buildings
        std::array<int, 25> building_counts;  // Count of each building type
        std::vector<MapPos> building_positions;
        std::vector<Building::Type> building_types;
        
        // Military
        std::array<int, 5> knight_counts;  // Knight0-Knight4
        int castle_knights;
        int knight_morale;
        
        // Territory
        std::vector<MapPos> owned_territory;
        int territory_size;
        
        // Economic stats
        int total_serfs;
        int idle_serfs;
        float economic_score;
        float military_score;
    };
    
    PlayerState self;  // This AI player's state
    std::vector<PlayerState> opponents;  // Other players' states
    
    // Map information
    struct MapInfo {
        int width;
        int height;
        
        // Terrain data (simplified for AI)
        std::vector<uint8_t> terrain_types;  // Per tile
        std::vector<uint8_t> elevation;      // Per tile
        std::vector<uint8_t> ownership;      // Per tile (player index)
        std::vector<bool> has_building;      // Per tile
        std::vector<bool> has_flag;          // Per tile
        std::vector<bool> has_road;          // Per tile
        
        // Resource deposits
        std::vector<MapPos> stone_deposits;
        std::vector<MapPos> coal_deposits;
        std::vector<MapPos> iron_deposits;
        std::vector<MapPos> gold_deposits;
        std::vector<MapPos> water_tiles;
    };
    
    MapInfo map;
    
    // Global game state
    struct GlobalInfo {
        int total_players;
        int active_players;
        bool game_ended;
        int winning_player;  // -1 if game ongoing
        
        // Performance info
        float ai_time_budget_ms;  // How much time this AI has this tick
        float last_execution_time_ms;  // How long last AI call took
    };
    
    GlobalInfo global;
};