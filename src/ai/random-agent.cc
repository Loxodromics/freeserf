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
    
    // Handle castle building first (if needed)
    if (!state.self.has_castle) {
        // Find a random position for castle
        MapPos castle_pos = get_random_position(state);
        if (castle_pos != 0) {
            actions.push_back(AIAction::build_castle(castle_pos));
            AILogger::log_debug(agent_name + ": Placing random castle at " + std::to_string(castle_pos));
            return actions;  // Only castle this tick
        }
    }
    
    int actions_this_tick = 0;
    
    // Random building placement
    if (should_place_building(state) && actions_this_tick < MAX_ACTIONS_PER_TICK) {
        Building::Type random_type = get_random_building_type();
        MapPos random_pos = get_random_position(state);
        
        if (random_pos != 0) {
            AIActionType action_type = building_type_to_action_type(random_type);
            
            switch (action_type) {
                case AIActionType::BUILD_FISHER:
                    actions.push_back(AIAction::build_fisher(random_pos));
                    break;
                case AIActionType::BUILD_BOATBUILDER:
                    actions.push_back(AIAction::build_boatbuilder(random_pos));
                    break;
                case AIActionType::BUILD_STONECUTTER:
                    actions.push_back(AIAction::build_stonecutter(random_pos));
                    break;
                case AIActionType::BUILD_STONE_MINE:
                    actions.push_back(AIAction::build_stone_mine(random_pos));
                    break;
                case AIActionType::BUILD_COAL_MINE:
                    actions.push_back(AIAction::build_coal_mine(random_pos));
                    break;
                case AIActionType::BUILD_IRON_MINE:
                    actions.push_back(AIAction::build_iron_mine(random_pos));
                    break;
                case AIActionType::BUILD_GOLD_MINE:
                    actions.push_back(AIAction::build_gold_mine(random_pos));
                    break;
                case AIActionType::BUILD_STOCK:
                    actions.push_back(AIAction::build_stock(random_pos));
                    break;
                case AIActionType::BUILD_HUT:
                    actions.push_back(AIAction::build_hut(random_pos));
                    break;
                case AIActionType::BUILD_FARM:
                    actions.push_back(AIAction::build_farm(random_pos));
                    break;
                case AIActionType::BUILD_BUTCHER:
                    actions.push_back(AIAction::build_butcher(random_pos));
                    break;
                case AIActionType::BUILD_PIG_FARM:
                    actions.push_back(AIAction::build_pig_farm(random_pos));
                    break;
                case AIActionType::BUILD_MILL:
                    actions.push_back(AIAction::build_mill(random_pos));
                    break;
                case AIActionType::BUILD_BAKER:
                    actions.push_back(AIAction::build_baker(random_pos));
                    break;
                case AIActionType::BUILD_SAWMILL:
                    actions.push_back(AIAction::build_sawmill(random_pos));
                    break;
                case AIActionType::BUILD_STEEL_SMELTER:
                    actions.push_back(AIAction::build_steel_smelter(random_pos));
                    break;
                case AIActionType::BUILD_TOOL_MAKER:
                    actions.push_back(AIAction::build_tool_maker(random_pos));
                    break;
                case AIActionType::BUILD_WEAPON_SMITH:
                    actions.push_back(AIAction::build_weapon_smith(random_pos));
                    break;
                case AIActionType::BUILD_TOWER:
                    actions.push_back(AIAction::build_tower(random_pos));
                    break;
                case AIActionType::BUILD_FORTRESS:
                    actions.push_back(AIAction::build_fortress(random_pos));
                    break;
                case AIActionType::BUILD_GOLD_SMELTER:
                    actions.push_back(AIAction::build_gold_smelter(random_pos));
                    break;
                case AIActionType::BUILD_LUMBERJACK:
                    actions.push_back(AIAction::build_lumberjack(random_pos));
                    break;
                case AIActionType::BUILD_FORESTER:
                    actions.push_back(AIAction::build_forester(random_pos));
                    break;
                default:
                    // Fallback for unmapped types
                    break;
            }
            
            if (!actions.empty()) {
                AILogger::log_debug(agent_name + ": Random building " + std::to_string(static_cast<int>(random_type)) + 
                                   " at " + std::to_string(random_pos));
                actions_this_tick++;
            }
        }
    }
    
    // Random road building
    if (should_build_roads(state) && actions_this_tick < MAX_ACTIONS_PER_TICK) {
        auto available_flags = get_available_flags(state);
        if (available_flags.size() >= 2) {
            std::uniform_int_distribution<size_t> flag_selector(0, available_flags.size() - 1);
            MapPos flag1 = available_flags[flag_selector(gen)];
            MapPos flag2 = available_flags[flag_selector(gen)];
            
            if (flag1 != flag2) {
                actions.push_back(AIAction::build_road(flag1, flag2));
                AILogger::log_debug(agent_name + ": Random road " + std::to_string(flag1) + " -> " + std::to_string(flag2));
                actions_this_tick++;
            }
        }
    }
    
    // Random flag placement (if no other actions)
    if (actions.empty() && prob_dist(gen) < 0.1f) {  // 10% chance for random flag
        MapPos random_pos = get_random_position(state);
        if (random_pos != 0) {
            actions.push_back(AIAction::build_flag(random_pos));
            AILogger::log_debug(agent_name + ": Random flag at " + std::to_string(random_pos));
        }
    }
    
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