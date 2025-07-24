#include "random-agent.h"
#include "ai-logger.h"
#include <algorithm>
#include <cmath>

RandomAgent::RandomAgent(const std::string& name) 
    : gen(rd()), prob_dist(0.0f, 1.0f), agent_name(name), difficulty_level(5), personality_type(0) {
    initialize_building_types();
    AILogger::log_debug("RandomAgent created: " + agent_name);
}

void RandomAgent::initialize_building_types() {
    // All building types except castle (handled specially)
    available_buildings = {
        Building::TypeFisher,
        Building::TypeLumberjack,
        Building::TypeBoatbuilder,
        Building::TypeStonecutter,
        Building::TypeStoneMine,
        Building::TypeCoalMine,
        Building::TypeIronMine,
        Building::TypeGoldMine,
        Building::TypeForester,
        Building::TypeStock,
        Building::TypeHut,
        Building::TypeFarm,
        Building::TypeButcher,
        Building::TypePigFarm,
        Building::TypeMill,
        Building::TypeBaker,
        Building::TypeSawmill,
        Building::TypeSteelSmelter,
        Building::TypeToolMaker,
        Building::TypeWeaponSmith,
        Building::TypeTower,
        Building::TypeFortress,
        Building::TypeGoldSmelter
    };
}

std::vector<AIAction> RandomAgent::get_actions(const GameState& state) {
    std::vector<AIAction> actions;
    
    // Log tick and state information for timing analysis
    AILogger::log_debug(agent_name + ": [TICK START] Tick " + std::to_string(state.game_tick) + 
                       " - awaiting: " + std::to_string(buildings_awaiting_connection.size()) + 
                       ", failed: " + std::to_string(buildings_failed_connection.size()) + 
                       ", has_flag.size: " + std::to_string(state.map.has_flag.size()));
    
    // Update internal state first
    update_building_states(state);
    
    // Handle castle building first (if needed)
    if (!state.self.has_castle) {
        MapPos castle_pos = get_random_position(state);
        if (castle_pos != 0) {
            actions.push_back(AIAction::build_castle(castle_pos));
            AILogger::log_debug(agent_name + ": Placing castle at " + std::to_string(castle_pos) + 
                               " [IMMEDIATE CONNECTION WILL BE SKIPPED - CASTLE IS ROOT]");
            return actions;  // Only castle this tick, no immediate connection needed
        }
    }
    
    int actions_this_tick = 0;
    
    // PRIORITY 1: Connect recently built buildings (highest priority)
    if (actions_this_tick < MAX_ACTIONS_PER_TICK && !buildings_awaiting_connection.empty()) {
        auto& pending = buildings_awaiting_connection[0];
        
        AILogger::log_debug(agent_name + ": [PRIORITY 1] Processing building at " + std::to_string(pending.position) + 
                           " (type: " + std::to_string(static_cast<int>(pending.type)) + 
                           ", flag: " + std::to_string(pending.flag_position) + 
                           ", built_tick: " + std::to_string(pending.built_tick) +
                           ", age: " + std::to_string(state.game_tick - pending.built_tick) + ")");
        
        // CRITICAL: First verify that the building actually exists (placement may have failed)
        // BUT: Allow grace period for GameState synchronization (buildings need 3-4 ticks to appear in state)
        uint32_t building_age = state.game_tick - pending.built_tick;
        if (building_age >= 4) { // Only check existence after grace period
            if (pending.position >= state.map.has_building.size() || !state.map.has_building[pending.position]) {
                AILogger::log_debug(agent_name + ": [BUILDING VALIDATION] Building at " + std::to_string(pending.position) + 
                                   " does not exist after grace period - placement failed (tick " + std::to_string(state.game_tick) + 
                                   ", age " + std::to_string(building_age) + "), removing from queue");
                // Remove from pending list - building placement definitely failed
                buildings_awaiting_connection.erase(buildings_awaiting_connection.begin());
                return actions; // Continue processing next tick
            } else {
                AILogger::log_debug(agent_name + ": [BUILDING VALIDATION] Building at " + std::to_string(pending.position) + 
                                   " exists after grace period (age " + std::to_string(building_age) + "), proceeding with connection");
            }
        } else {
            AILogger::log_debug(agent_name + ": [GAMESTATE SYNC] Building at " + std::to_string(pending.position) + 
                               " still in grace period (age " + std::to_string(building_age) + 
                               "), waiting for GameState synchronization");
            return actions; // Wait for GameState to synchronize
        }
        
        // CRITICAL: Verify that the building's flag actually exists before attempting connection
        if (pending.flag_position >= state.map.has_flag.size() || !state.map.has_flag[pending.flag_position]) {
            AILogger::log_debug(agent_name + ": [FLAG VALIDATION] Building flag at " + std::to_string(pending.flag_position) + 
                               " does not exist yet (tick " + std::to_string(state.game_tick) + 
                               ", age " + std::to_string(state.game_tick - pending.built_tick) + "), deferring connection");
            return actions; // Wait for flag to be created
        }
        
        AILogger::log_debug(agent_name + ": [PRIORITY 1] Flag at " + std::to_string(pending.flag_position) + " exists, searching for connection target");
        MapPos target_flag = find_connection_target_flag(state, pending.flag_position);
        
        if (target_flag != 0 && target_flag != pending.flag_position) {
            // Connection possible - build road
            actions.push_back(AIAction::build_road(pending.flag_position, target_flag));
            AILogger::log_debug(agent_name + ": [PRIORITY 1] Connecting pending building at " + 
                               std::to_string(pending.position) + " (flag: " + std::to_string(pending.flag_position) + 
                               " -> " + std::to_string(target_flag) + ")");
            actions_this_tick++;
            // Remove from pending list on attempt
            buildings_awaiting_connection.erase(buildings_awaiting_connection.begin());
        } else {
            // No connection possible - move to failed list
            AILogger::log_debug(agent_name + ": [PRIORITY 1] No connection target for building at " + 
                               std::to_string(pending.position) + ", moving to demolition queue");
            move_to_failed_connection(pending);
            buildings_awaiting_connection.erase(buildings_awaiting_connection.begin());
        }
    }
    
    // PRIORITY 2: Demolish buildings that failed connection (but never castles)
    if (actions_this_tick < MAX_ACTIONS_PER_TICK && !buildings_failed_connection.empty()) {
        auto& failed = buildings_failed_connection[0];
        
        if (!is_castle_building(failed.type)) {
            // Safe to demolish non-castle building
            actions.push_back(AIAction::demolish_building(failed.position));
            AILogger::log_debug(agent_name + ": [PRIORITY 2] Demolishing unconnectable building at " + 
                               std::to_string(failed.position) + " (type: " + 
                               std::to_string(static_cast<int>(failed.type)) + ")");
            actions_this_tick++;
        } else {
            // Castle protection - never demolish castles
            AILogger::log_debug(agent_name + ": [CASTLE PROTECTION] Refusing to demolish castle at " + 
                               std::to_string(failed.position) + " - castle preserved");
        }
        
        // Remove from failed list regardless of whether we demolished it
        buildings_failed_connection.erase(buildings_failed_connection.begin());
    }
    
    // PRIORITY 3: Place new buildings (one action per tick)
    if (actions_this_tick < MAX_ACTIONS_PER_TICK && should_place_building(state)) {
        Building::Type random_type = get_random_building_type();
        MapPos random_pos = get_random_position(state);
        
        AILogger::log_debug(agent_name + ": [PRIORITY 3] Attempting to place building type " + 
                           std::to_string(static_cast<int>(random_type)) + " at position " + 
                           std::to_string(random_pos) + " on tick " + std::to_string(state.game_tick));
        
        if (random_pos != 0) {
            // Place building only - connection will be attempted next tick
            actions.push_back(create_building_action(random_type, random_pos));
            
            // Calculate flag position and add to pending connection queue
            // NOTE: This assumes building placement will succeed - if it fails, the flag won't exist
            MapPos flag_pos = calculate_building_flag_position(random_pos, random_type, state);
            if (flag_pos != 0) {
                add_pending_building(random_pos, flag_pos, random_type, state.game_tick);
                AILogger::log_debug(agent_name + ": [CRITICAL TIMING] Building queued optimistically - " + 
                                   "type " + std::to_string(static_cast<int>(random_type)) + 
                                   " at " + std::to_string(random_pos) + 
                                   " (estimated flag: " + std::to_string(flag_pos) + 
                                   ") - SUCCESS UNKNOWN UNTIL NEXT TICK");
            } else {
                AILogger::log_debug(agent_name + ": [WARNING] Cannot calculate flag position for building " + 
                                   std::to_string(static_cast<int>(random_type)) + " at " + 
                                   std::to_string(random_pos) + " - connection will fail");
            }
            actions_this_tick++;
        } else {
            AILogger::log_debug(agent_name + ": [PRIORITY 3] No valid position found for building type " + 
                               std::to_string(static_cast<int>(random_type)));
        }
    }
    
    // Fallback: Random flag placement (if no other actions)
    if (actions.empty() && prob_dist(gen) < 0.1f) {  // 10% chance for random flag
        MapPos random_pos = get_random_position(state);
        if (random_pos != 0) {
            actions.push_back(AIAction::build_flag(random_pos));
            AILogger::log_debug(agent_name + ": [FALLBACK] Random flag at " + std::to_string(random_pos));
        }
    }
    
    // Log final decision summary
    AILogger::log_debug(agent_name + ": [TICK END] Tick " + std::to_string(state.game_tick) + 
                       " - Returning " + std::to_string(actions.size()) + " action(s), " +
                       std::to_string(buildings_awaiting_connection.size()) + " buildings still awaiting connection");
    
    return actions;
}

bool RandomAgent::should_place_building(const GameState& state) {
    return prob_dist(gen) < BASE_BUILDING_PROBABILITY;
}

bool RandomAgent::should_build_roads(const GameState& state) {
    return prob_dist(gen) < BASE_ROAD_PROBABILITY;
}

Building::Type RandomAgent::get_random_building_type() {
    if (available_buildings.empty()) {
        return Building::TypeLumberjack;  // Fallback
    }
    
    std::uniform_int_distribution<size_t> building_selector(0, available_buildings.size() - 1);
    return available_buildings[building_selector(gen)];
}

MapPos RandomAgent::get_random_position(const GameState& state) {
    if (state.map.width == 0 || state.map.height == 0) {
        return 0;
    }
    
    std::uniform_int_distribution<MapPos> pos_selector(0, state.map.width * state.map.height - 1);
    
    // Try up to 50 random positions to find a reasonable one
    for (int attempts = 0; attempts < 50; ++attempts) {
        MapPos pos = pos_selector(gen);
        
        // Basic sanity check - avoid edge positions
        int x = pos % state.map.width;
        int y = pos / state.map.width;
        
        if (x > 5 && x < static_cast<int>(state.map.width) - 5 && 
            y > 5 && y < static_cast<int>(state.map.height) - 5) {
            return pos;
        }
    }
    
    // Fallback to center if no good position found
    return (state.map.height / 2) * state.map.width + (state.map.width / 2);
}

std::vector<MapPos> RandomAgent::get_owned_positions(const GameState& state) {
    std::vector<MapPos> owned;
    // This would need to scan the ownership map, but for random placement we'll use random positions
    return owned;
}

std::vector<MapPos> RandomAgent::get_available_flags(const GameState& state) {
    std::vector<MapPos> flags;
    
    // For now, return a few random positions that might have flags
    // In a real implementation, this would scan the map for actual flag positions
    for (int i = 0; i < 10; ++i) {  // Generate 10 potential flag positions
        MapPos pos = get_random_position(state);
        if (pos != 0) {
            flags.push_back(pos);
        }
    }
    
    return flags;
}

AIActionType RandomAgent::building_type_to_action_type(Building::Type type) {
    switch (type) {
        case Building::TypeFisher: return AIActionType::BUILD_FISHER;
        case Building::TypeLumberjack: return AIActionType::BUILD_LUMBERJACK;
        case Building::TypeBoatbuilder: return AIActionType::BUILD_BOATBUILDER;
        case Building::TypeStonecutter: return AIActionType::BUILD_STONECUTTER;
        case Building::TypeStoneMine: return AIActionType::BUILD_STONE_MINE;
        case Building::TypeCoalMine: return AIActionType::BUILD_COAL_MINE;
        case Building::TypeIronMine: return AIActionType::BUILD_IRON_MINE;
        case Building::TypeGoldMine: return AIActionType::BUILD_GOLD_MINE;
        case Building::TypeForester: return AIActionType::BUILD_FORESTER;
        case Building::TypeStock: return AIActionType::BUILD_STOCK;
        case Building::TypeHut: return AIActionType::BUILD_HUT;
        case Building::TypeFarm: return AIActionType::BUILD_FARM;
        case Building::TypeButcher: return AIActionType::BUILD_BUTCHER;
        case Building::TypePigFarm: return AIActionType::BUILD_PIG_FARM;
        case Building::TypeMill: return AIActionType::BUILD_MILL;
        case Building::TypeBaker: return AIActionType::BUILD_BAKER;
        case Building::TypeSawmill: return AIActionType::BUILD_SAWMILL;
        case Building::TypeSteelSmelter: return AIActionType::BUILD_STEEL_SMELTER;
        case Building::TypeToolMaker: return AIActionType::BUILD_TOOL_MAKER;
        case Building::TypeWeaponSmith: return AIActionType::BUILD_WEAPON_SMITH;
        case Building::TypeTower: return AIActionType::BUILD_TOWER;
        case Building::TypeFortress: return AIActionType::BUILD_FORTRESS;
        case Building::TypeGoldSmelter: return AIActionType::BUILD_GOLD_SMELTER;
        case Building::TypeCastle: return AIActionType::BUILD_CASTLE;
        default: return AIActionType::BUILD_LUMBERJACK;  // Fallback
    }
}

int RandomAgent::get_action_space_size() const {
    return 26;  // Total number of action types (BUILD_CASTLE through BUILD_GOLD_SMELTER + specials)
}

std::vector<bool> RandomAgent::get_valid_actions(const GameState& state) {
    // For RandomAgent, we'll mark all building actions as potentially valid
    // The actual validation happens during execution
    std::vector<bool> valid_actions(get_action_space_size(), true);
    return valid_actions;
}

void RandomAgent::set_difficulty(int difficulty) {
    difficulty_level = std::clamp(difficulty, 0, 10);
}

void RandomAgent::set_personality(int personality) {
    personality_type = std::clamp(personality, 0, 10);
}

AgentType RandomAgent::get_agent_type() const {
    return AgentType::RANDOM;
}

std::string RandomAgent::get_agent_name() const {
    return agent_name;
}

bool RandomAgent::is_ready() const {
    return true;  // RandomAgent is always ready
}

// Flag discovery system implementation for immediate road building

MapPos RandomAgent::calculate_building_flag_position(MapPos building_pos, Building::Type type, const GameState& state) {
    // For RandomAgent, we can't access Game object directly, so we estimate flag position
    // Based on research: flags are typically placed adjacent to buildings
    // This is a simplified approach - ideally we would query the game after building placement
    
    const auto& map = state.map;
    if (map.width == 0 || map.height == 0) {
        return 0;
    }
    
    int x = building_pos % map.width;
    int y = building_pos / map.width;
    
    // Try common flag positions relative to building (down-right is typical)
    std::vector<std::pair<int, int>> offsets = {
        {1, 1},   // down-right (most common)
        {1, 0},   // right
        {0, 1},   // down
        {-1, 1},  // down-left
        {1, -1},  // up-right
        {0, -1}   // up
    };
    
    for (const auto& offset : offsets) {
        int new_x = x + offset.first;
        int new_y = y + offset.second;
        
        if (new_x >= 0 && new_x < static_cast<int>(map.width) && 
            new_y >= 0 && new_y < static_cast<int>(map.height)) {
            
            MapPos pos = new_y * map.width + new_x;
            
            // Check if position is likely suitable for flag (simple heuristic)
            if (pos < map.has_flag.size() && !map.has_building[pos]) {
                return pos;
            }
        }
    }
    
    return 0; // No suitable position found
}

std::vector<MapPos> RandomAgent::find_all_player_flags(const GameState& state) {
    std::vector<MapPos> flags;
    const auto& map = state.map;
    
    AILogger::log_debug(agent_name + ": [FLAG SCAN] Starting flag scan - map size: " + 
                       std::to_string(map.width) + "x" + std::to_string(map.height) + 
                       ", has_flag.size: " + std::to_string(map.has_flag.size()) + 
                       ", ownership.size: " + std::to_string(map.ownership.size()) + 
                       ", player_index: " + std::to_string(state.self.player_index));
    
    int total_flags_found = 0;
    int player_owned_flags = 0;
    
    // Scan the has_flag boolean vector to find all flag positions
    for (size_t i = 0; i < map.has_flag.size(); ++i) {
        if (map.has_flag[i]) {
            total_flags_found++;
            // Check if this flag belongs to our player by checking nearby ownership
            if (i < map.ownership.size() && map.ownership[i] == state.self.player_index) {
                player_owned_flags++;
                flags.push_back(static_cast<MapPos>(i));
                int x = i % map.width;
                int y = i / map.width;
                AILogger::log_debug(agent_name + ": [FLAG SCAN] Player flag found at pos " + 
                                   std::to_string(i) + " (" + std::to_string(x) + "," + std::to_string(y) + ")");
            } else {
                // Log other player flags for debugging
                int owner = (i < map.ownership.size()) ? map.ownership[i] : -1;
                AILogger::log_debug(agent_name + ": [FLAG SCAN] Other flag at pos " + std::to_string(i) + 
                                   " owned by player " + std::to_string(owner));
            }
        }
    }
    
    AILogger::log_debug(agent_name + ": [FLAG SCAN] Summary - Total flags: " + std::to_string(total_flags_found) + 
                       ", Player flags: " + std::to_string(player_owned_flags) + 
                       ", Returned: " + std::to_string(flags.size()));
    return flags;
}

MapPos RandomAgent::find_castle_flag(const GameState& state) {
    AILogger::log_debug(agent_name + ": Looking for castle - building_positions.size=" + 
                       std::to_string(state.self.building_positions.size()) + 
                       ", building_types.size=" + std::to_string(state.self.building_types.size()));
    
    // Method 1: Look for castle in building positions array
    for (size_t i = 0; i < state.self.building_positions.size(); ++i) {
        if (state.self.building_types[i] == Building::TypeCastle) {
            MapPos castle_pos = state.self.building_positions[i];
            AILogger::log_debug(agent_name + ": Castle found in array at position " + std::to_string(castle_pos));
            
            // Use area scanning to find the actual flag near castle position
            MapPos castle_flag = find_actual_flag_near_position(castle_pos, state, 5); // 5-tile radius search
            if (castle_flag != 0) {
                AILogger::log_debug(agent_name + ": Castle flag found at " + std::to_string(castle_flag) + 
                                   " (near castle at " + std::to_string(castle_pos) + ")");
                return castle_flag;
            } else {
                AILogger::log_debug(agent_name + ": Castle at " + std::to_string(castle_pos) + 
                                   " has no flag in 5-tile radius - may still be under construction");
            }
        }
    }
    
    // Method 2: Fallback - scan the map for flags near buildings
    AILogger::log_debug(agent_name + ": No castle in building array, scanning map for potential castle flags");
    const auto& map = state.map;
    
    // Look for flags in areas where we own territory (likely near our castle)
    std::vector<MapPos> potential_castle_flags;
    for (size_t i = 0; i < map.has_flag.size() && i < map.ownership.size(); ++i) {
        if (map.has_flag[i] && map.ownership[i] == state.self.player_index) {
            // Check if this flag is in a central area of our territory (potential castle location)
            int nearby_owned_tiles = 0;
            int x = i % map.width;
            int y = i / map.width;
            
            // Count owned tiles in 5x5 area around flag
            for (int dx = -2; dx <= 2; dx++) {
                for (int dy = -2; dy <= 2; dy++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < static_cast<int>(map.width) && 
                        ny >= 0 && ny < static_cast<int>(map.height)) {
                        MapPos pos = ny * map.width + nx;
                        if (pos < map.ownership.size() && map.ownership[pos] == state.self.player_index) {
                            nearby_owned_tiles++;
                        }
                    }
                }
            }
            
            // If this flag has many owned tiles around it, it's likely the castle flag
            if (nearby_owned_tiles >= 15) { // At least 15/25 tiles owned
                potential_castle_flags.push_back(i);
                AILogger::log_debug(agent_name + ": Potential castle flag at " + std::to_string(i) + 
                                   " (owned_tiles: " + std::to_string(nearby_owned_tiles) + ")");
            }
        }
    }
    
    // Return the first potential castle flag found
    if (!potential_castle_flags.empty()) {
        MapPos castle_flag = potential_castle_flags[0];
        AILogger::log_debug(agent_name + ": Using fallback castle flag at " + std::to_string(castle_flag));
        return castle_flag;
    }
    
    AILogger::log_debug(agent_name + ": No castle flag found via any method");
    return 0;
}

MapPos RandomAgent::find_nearest_flag(const GameState& state, MapPos target_pos) {
    std::vector<MapPos> available_flags = find_all_player_flags(state);
    
    if (available_flags.empty()) {
        return 0;
    }
    
    MapPos nearest = available_flags[0];
    int min_distance = calculate_distance(target_pos, nearest, state);
    
    for (MapPos flag_pos : available_flags) {
        int distance = calculate_distance(target_pos, flag_pos, state);
        if (distance < min_distance) {
            min_distance = distance;
            nearest = flag_pos;
        }
    }
    
    AILogger::log_debug(agent_name + ": Nearest flag at " + std::to_string(nearest) + 
                       " (distance: " + std::to_string(min_distance) + ")");
    return nearest;
}

int RandomAgent::calculate_distance(MapPos pos1, MapPos pos2, const GameState& state) {
    const auto& map = state.map;
    if (map.width == 0) {
        return 1000; // Large distance if invalid map
    }
    
    int x1 = pos1 % map.width;
    int y1 = pos1 / map.width;
    int x2 = pos2 % map.width;
    int y2 = pos2 / map.width;
    
    // Manhattan distance - simple and fast
    return abs(x1 - x2) + abs(y1 - y2);
}

MapPos RandomAgent::find_connection_target_flag(const GameState& state, MapPos new_flag_pos) {
    // Priority 1: Connect to castle flag if available and castle has few connections
    MapPos castle_flag = find_castle_flag(state);
    if (castle_flag != 0) {
        int distance = calculate_distance(new_flag_pos, castle_flag, state);
        int castle_connections = count_castle_connections(state);
        
        // Prioritize castle connections especially if castle is isolated
        bool should_connect_to_castle = false;
        if (castle_connections == 0) {
            // Castle is isolated - connect even if far away
            should_connect_to_castle = (distance <= 30);
            AILogger::log_debug(agent_name + ": Castle is isolated (" + std::to_string(castle_connections) + 
                               " connections), prioritizing connection");
        } else if (castle_connections < 3) {
            // Castle has few connections - normal priority
            should_connect_to_castle = (distance <= 20);
            AILogger::log_debug(agent_name + ": Castle has " + std::to_string(castle_connections) + 
                               " connections, normal priority");
        } else {
            // Castle is well connected - lower priority
            should_connect_to_castle = (distance <= 12);
            AILogger::log_debug(agent_name + ": Castle has " + std::to_string(castle_connections) + 
                               " connections, lower priority");
        }
        
        if (should_connect_to_castle) {
            AILogger::log_debug(agent_name + ": Target: castle flag at " + std::to_string(castle_flag) + 
                               " (distance: " + std::to_string(distance) + 
                               ", connections: " + std::to_string(castle_connections) + ") [CASTLE PRIORITY]");
            return castle_flag;
        } else {
            AILogger::log_debug(agent_name + ": Castle flag at " + std::to_string(castle_flag) + 
                               " too far (distance: " + std::to_string(distance) + 
                               ", connections: " + std::to_string(castle_connections) + ")");
        }
    }
    
    // Priority 2: Connect to nearest existing flag
    MapPos nearest_flag = find_nearest_flag(state, new_flag_pos);
    if (nearest_flag != 0 && nearest_flag != castle_flag) {
        int distance = calculate_distance(new_flag_pos, nearest_flag, state);
        if (distance <= 15) { // Reasonable distance limit
            AILogger::log_debug(agent_name + ": Target: nearest flag at " + std::to_string(nearest_flag) + 
                               " (distance: " + std::to_string(distance) + ") [NEAREST]");
            return nearest_flag;
        }
    }
    
    AILogger::log_debug(agent_name + ": No suitable connection target found");
    return 0; // No suitable target found
}

// Robust castle flag detection system implementation

MapPos RandomAgent::find_actual_flag_near_position(MapPos center, const GameState& state, int radius) {
    const auto& map = state.map;
    if (map.width == 0 || map.height == 0 || center >= map.has_flag.size()) {
        return 0;
    }
    
    int center_x = center % map.width;
    int center_y = center / map.width;
    int flags_scanned = 0;
    MapPos closest_flag = 0;
    int closest_distance = 1000;
    
    AILogger::log_debug(agent_name + ": Scanning for flags near position " + std::to_string(center) + 
                       " (" + std::to_string(center_x) + "," + std::to_string(center_y) + ") radius=" + std::to_string(radius));
    
    // Scan in expanding radius around center position
    for (int r = 1; r <= radius; r++) {
        for (int dx = -r; dx <= r; dx++) {
            for (int dy = -r; dy <= r; dy++) {
                // Only check positions at current radius (not inner positions already checked)
                if (abs(dx) != r && abs(dy) != r) continue;
                
                int x = center_x + dx;
                int y = center_y + dy;
                
                // Bounds check
                if (x < 0 || x >= static_cast<int>(map.width) || 
                    y < 0 || y >= static_cast<int>(map.height)) {
                    continue;
                }
                
                MapPos pos = y * map.width + x;
                if (pos >= map.has_flag.size()) continue;
                
                flags_scanned++;
                
                // Check if there's a flag at this position
                if (map.has_flag[pos]) {
                    // Verify ownership - check if this flag belongs to our player
                    if (pos < map.ownership.size() && map.ownership[pos] == state.self.player_index) {
                        int distance = abs(dx) + abs(dy); // Manhattan distance
                        if (distance < closest_distance) {
                            closest_distance = distance;
                            closest_flag = pos;
                        }
                        AILogger::log_debug(agent_name + ": Found player flag at " + std::to_string(pos) + 
                                           " (" + std::to_string(x) + "," + std::to_string(y) + 
                                           ") distance=" + std::to_string(distance));
                    }
                }
            }
        }
    }
    
    if (closest_flag != 0) {
        AILogger::log_debug(agent_name + ": Closest flag at " + std::to_string(closest_flag) + 
                           " (distance: " + std::to_string(closest_distance) + 
                           ", flags_scanned: " + std::to_string(flags_scanned) + ")");
    } else {
        AILogger::log_debug(agent_name + ": No flags found in radius " + std::to_string(radius) + 
                           " (flags_scanned: " + std::to_string(flags_scanned) + ")");
    }
    
    return closest_flag;
}

bool RandomAgent::is_castle_connected(const GameState& state) {
    return count_castle_connections(state) > 0;
}

int RandomAgent::count_castle_connections(const GameState& state) {
    MapPos castle_flag = find_castle_flag(state);
    if (castle_flag == 0) {
        return 0;
    }
    
    // Count flags connected to castle flag by checking nearby road connections
    // This is a simplified check - in a full implementation we'd trace actual road paths
    std::vector<MapPos> all_flags = find_all_player_flags(state);
    int connections = 0;
    
    for (MapPos flag_pos : all_flags) {
        if (flag_pos != castle_flag) {
            int distance = calculate_distance(castle_flag, flag_pos, state);
            if (distance <= 3) { // Assume close flags might be connected
                connections++;
            }
        }
    }
    
    AILogger::log_debug(agent_name + ": Castle connections estimated: " + std::to_string(connections));
    return connections;
}

// Sequential building->road->demolish state management methods

void RandomAgent::update_building_states(const GameState& state) {
    // Clean up buildings that no longer exist or are already connected
    clear_completed_buildings(state);
    
    // Log current state for debugging
    if (!buildings_awaiting_connection.empty() || !buildings_failed_connection.empty()) {
        AILogger::log_debug(agent_name + ": State update - awaiting: " + 
                           std::to_string(buildings_awaiting_connection.size()) + 
                           ", failed: " + std::to_string(buildings_failed_connection.size()));
    }
}

void RandomAgent::add_pending_building(MapPos building_pos, MapPos flag_pos, 
                                      Building::Type type, uint32_t tick) {
    buildings_awaiting_connection.emplace_back(building_pos, flag_pos, type, tick);
    AILogger::log_debug(agent_name + ": Added building to connection queue - pos: " + 
                       std::to_string(building_pos) + ", flag: " + std::to_string(flag_pos) + 
                       ", type: " + std::to_string(static_cast<int>(type)));
}

void RandomAgent::move_to_failed_connection(const PendingBuilding& building) {
    buildings_failed_connection.push_back(building);
    AILogger::log_debug(agent_name + ": Building connection failed, moved to demolition queue - pos: " + 
                       std::to_string(building.position) + ", type: " + 
                       std::to_string(static_cast<int>(building.type)));
}

bool RandomAgent::is_castle_building(Building::Type type) {
    return type == Building::TypeCastle;
}

void RandomAgent::clear_completed_buildings(const GameState& state) {
    // Remove buildings from pending list if they no longer exist in the game
    // This handles cases where buildings were demolished by other means or external events
    
    auto remove_from_awaiting = std::remove_if(buildings_awaiting_connection.begin(), 
                                              buildings_awaiting_connection.end(),
        [&](const PendingBuilding& building) {
            // Simple check: if building is very old (>10 ticks), assume it's been processed
            uint32_t age = state.game_tick - building.built_tick;
            if (age > 10) {
                AILogger::log_debug(agent_name + ": Removing stale building from awaiting queue: " + 
                                   std::to_string(building.position) + " (age: " + std::to_string(age) + ")");
                return true;
            }
            return false;
        });
    
    buildings_awaiting_connection.erase(remove_from_awaiting, buildings_awaiting_connection.end());
    
    // Clear failed connection list after a reasonable timeout to prevent infinite accumulation
    auto remove_from_failed = std::remove_if(buildings_failed_connection.begin(), 
                                            buildings_failed_connection.end(),
        [&](const PendingBuilding& building) {
            uint32_t age = state.game_tick - building.built_tick;
            if (age > 20) {  // Keep failed buildings longer for demolition attempts
                AILogger::log_debug(agent_name + ": Removing stale building from failed queue: " + 
                                   std::to_string(building.position) + " (age: " + std::to_string(age) + ")");
                return true;
            }
            return false;
        });
    
    buildings_failed_connection.erase(remove_from_failed, buildings_failed_connection.end());
}

// Helper method to create building action from type and position
AIAction RandomAgent::create_building_action(Building::Type building_type, MapPos pos) {
    AIActionType action_type = building_type_to_action_type(building_type);
    
    switch (action_type) {
        case AIActionType::BUILD_FISHER:
            return AIAction::build_fisher(pos);
        case AIActionType::BUILD_BOATBUILDER:
            return AIAction::build_boatbuilder(pos);
        case AIActionType::BUILD_STONECUTTER:
            return AIAction::build_stonecutter(pos);
        case AIActionType::BUILD_STONE_MINE:
            return AIAction::build_stone_mine(pos);
        case AIActionType::BUILD_COAL_MINE:
            return AIAction::build_coal_mine(pos);
        case AIActionType::BUILD_IRON_MINE:
            return AIAction::build_iron_mine(pos);
        case AIActionType::BUILD_GOLD_MINE:
            return AIAction::build_gold_mine(pos);
        case AIActionType::BUILD_STOCK:
            return AIAction::build_stock(pos);
        case AIActionType::BUILD_HUT:
            return AIAction::build_hut(pos);
        case AIActionType::BUILD_FARM:
            return AIAction::build_farm(pos);
        case AIActionType::BUILD_BUTCHER:
            return AIAction::build_butcher(pos);
        case AIActionType::BUILD_PIG_FARM:
            return AIAction::build_pig_farm(pos);
        case AIActionType::BUILD_MILL:
            return AIAction::build_mill(pos);
        case AIActionType::BUILD_BAKER:
            return AIAction::build_baker(pos);
        case AIActionType::BUILD_SAWMILL:
            return AIAction::build_sawmill(pos);
        case AIActionType::BUILD_STEEL_SMELTER:
            return AIAction::build_steel_smelter(pos);
        case AIActionType::BUILD_TOOL_MAKER:
            return AIAction::build_tool_maker(pos);
        case AIActionType::BUILD_WEAPON_SMITH:
            return AIAction::build_weapon_smith(pos);
        case AIActionType::BUILD_TOWER:
            return AIAction::build_tower(pos);
        case AIActionType::BUILD_FORTRESS:
            return AIAction::build_fortress(pos);
        case AIActionType::BUILD_GOLD_SMELTER:
            return AIAction::build_gold_smelter(pos);
        case AIActionType::BUILD_LUMBERJACK:
            return AIAction::build_lumberjack(pos);
        case AIActionType::BUILD_FORESTER:
            return AIAction::build_forester(pos);
        case AIActionType::BUILD_CASTLE:
            return AIAction::build_castle(pos);
        default:
            // Fallback for unmapped types
            return AIAction::build_lumberjack(pos);
    }
}