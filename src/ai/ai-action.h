#pragma once

#include "../map.h"
#include "../building.h"
#include "../resource.h"

/**
 * AI Action Types for Phase 1
 * Will be expanded in later phases
 */
enum class AIActionType {
    // Phase 1 Actions (existing)
    BUILD_CASTLE = 0,
    BUILD_FLAG = 1,
    BUILD_ROAD = 2,
    BUILD_LUMBERJACK = 3,
    BUILD_FORESTER = 4,
    
    // Phase 0.8.1 New Building Actions
    BUILD_FISHER = 5,
    BUILD_BOATBUILDER = 6,
    BUILD_STONECUTTER = 7,
    BUILD_STONE_MINE = 8,
    BUILD_COAL_MINE = 9,
    BUILD_IRON_MINE = 10,
    BUILD_GOLD_MINE = 11,
    BUILD_STOCK = 12,
    BUILD_HUT = 13,
    BUILD_FARM = 14,
    BUILD_BUTCHER = 15,
    BUILD_PIG_FARM = 16,
    BUILD_MILL = 17,
    BUILD_BAKER = 18,
    BUILD_SAWMILL = 19,
    BUILD_STEEL_SMELTER = 20,
    BUILD_TOOL_MAKER = 21,
    BUILD_WEAPON_SMITH = 22,
    BUILD_TOWER = 23,
    BUILD_FORTRESS = 24,
    BUILD_GOLD_SMELTER = 25,
    
    // Demolition Actions
    DEMOLISH_BUILDING = 50,
    DEMOLISH_FLAG = 51,
    DEMOLISH_ROAD = 52,
    
    // Resource Priority Actions (60-85) - 26 resource types
    SET_RESOURCE_PRIORITY_FISH = 60,
    SET_RESOURCE_PRIORITY_PIG = 61,
    SET_RESOURCE_PRIORITY_MEAT = 62,
    SET_RESOURCE_PRIORITY_WHEAT = 63,
    SET_RESOURCE_PRIORITY_FLOUR = 64,
    SET_RESOURCE_PRIORITY_BREAD = 65,
    SET_RESOURCE_PRIORITY_LUMBER = 66,
    SET_RESOURCE_PRIORITY_PLANK = 67,
    SET_RESOURCE_PRIORITY_BOAT = 68,
    SET_RESOURCE_PRIORITY_STONE = 69,
    SET_RESOURCE_PRIORITY_IRON_ORE = 70,
    SET_RESOURCE_PRIORITY_STEEL = 71,
    SET_RESOURCE_PRIORITY_COAL = 72,
    SET_RESOURCE_PRIORITY_GOLD_ORE = 73,
    SET_RESOURCE_PRIORITY_GOLD_BAR = 74,
    SET_RESOURCE_PRIORITY_SHOVEL = 75,
    SET_RESOURCE_PRIORITY_HAMMER = 76,
    SET_RESOURCE_PRIORITY_ROD = 77,
    SET_RESOURCE_PRIORITY_CLEAVER = 78,
    SET_RESOURCE_PRIORITY_SCYTHE = 79,
    SET_RESOURCE_PRIORITY_AXE = 80,
    SET_RESOURCE_PRIORITY_SAW = 81,
    SET_RESOURCE_PRIORITY_PICK = 82,
    SET_RESOURCE_PRIORITY_PINCER = 83,
    SET_RESOURCE_PRIORITY_SWORD = 84,
    SET_RESOURCE_PRIORITY_SHIELD = 85,
    
    // Tool Priority Actions (90-98) - 9 tool types
    SET_TOOL_PRIORITY_SHOVEL = 90,
    SET_TOOL_PRIORITY_HAMMER = 91,
    SET_TOOL_PRIORITY_ROD = 92,
    SET_TOOL_PRIORITY_CLEAVER = 93,
    SET_TOOL_PRIORITY_SCYTHE = 94,
    SET_TOOL_PRIORITY_AXE = 95,
    SET_TOOL_PRIORITY_SAW = 96,
    SET_TOOL_PRIORITY_PICK = 97,
    SET_TOOL_PRIORITY_PINCER = 98,
    
    // Special Actions
    NO_ACTION = 99,
    WAIT = 100,
    
    // Food Distribution Actions (101-104) - 4 mine types
    SET_FOOD_STONE_MINE = 101,
    SET_FOOD_COAL_MINE = 102,
    SET_FOOD_IRON_MINE = 103,
    SET_FOOD_GOLD_MINE = 104,
    
    // Future expansion placeholder
    FUTURE_ACTION_START = 200
};

/**
 * AI Action Representation
 * Compact structure for representing AI decisions
 */
struct AIAction {
    AIActionType type;
    
    // Spatial parameters
    MapPos primary_position;    // Main target position
    MapPos secondary_position;  // Secondary position (for roads: destination)
    
    // Action parameters
    int parameter1;  // Action-specific parameter
    int parameter2;  // Additional parameter if needed
    
    // Execution metadata
    float priority;      // 0.0-1.0, higher = more important
    float confidence;    // 0.0-1.0, AI's confidence in this action
    
    // Constructors for common actions
    static AIAction build_castle(MapPos pos, float priority = 1.0f) {
        return {AIActionType::BUILD_CASTLE, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_flag(MapPos pos, float priority = 0.8f) {
        return {AIActionType::BUILD_FLAG, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_road(MapPos from, MapPos to, float priority = 0.9f) {
        return {AIActionType::BUILD_ROAD, from, to, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_lumberjack(MapPos pos, float priority = 0.7f) {
        return {AIActionType::BUILD_LUMBERJACK, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_forester(MapPos pos, float priority = 0.6f) {
        return {AIActionType::BUILD_FORESTER, pos, 0, 0, 0, priority, 1.0f};
    }
    
    // Phase 0.8.1 New Building Factory Methods
    static AIAction build_fisher(MapPos pos, float priority = 0.6f) {
        return {AIActionType::BUILD_FISHER, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_boatbuilder(MapPos pos, float priority = 0.6f) {
        return {AIActionType::BUILD_BOATBUILDER, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_stonecutter(MapPos pos, float priority = 0.7f) {
        return {AIActionType::BUILD_STONECUTTER, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_stone_mine(MapPos pos, float priority = 0.8f) {
        return {AIActionType::BUILD_STONE_MINE, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_coal_mine(MapPos pos, float priority = 0.8f) {
        return {AIActionType::BUILD_COAL_MINE, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_iron_mine(MapPos pos, float priority = 0.8f) {
        return {AIActionType::BUILD_IRON_MINE, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_gold_mine(MapPos pos, float priority = 0.9f) {
        return {AIActionType::BUILD_GOLD_MINE, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_stock(MapPos pos, float priority = 0.4f) {
        return {AIActionType::BUILD_STOCK, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_hut(MapPos pos, float priority = 0.6f) {
        return {AIActionType::BUILD_HUT, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_farm(MapPos pos, float priority = 0.6f) {
        return {AIActionType::BUILD_FARM, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_butcher(MapPos pos, float priority = 0.6f) {
        return {AIActionType::BUILD_BUTCHER, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_pig_farm(MapPos pos, float priority = 0.6f) {
        return {AIActionType::BUILD_PIG_FARM, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_mill(MapPos pos, float priority = 0.6f) {
        return {AIActionType::BUILD_MILL, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_baker(MapPos pos, float priority = 0.6f) {
        return {AIActionType::BUILD_BAKER, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_sawmill(MapPos pos, float priority = 0.7f) {
        return {AIActionType::BUILD_SAWMILL, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_steel_smelter(MapPos pos, float priority = 0.7f) {
        return {AIActionType::BUILD_STEEL_SMELTER, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_tool_maker(MapPos pos, float priority = 0.7f) {
        return {AIActionType::BUILD_TOOL_MAKER, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_weapon_smith(MapPos pos, float priority = 0.7f) {
        return {AIActionType::BUILD_WEAPON_SMITH, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_tower(MapPos pos, float priority = 0.8f) {
        return {AIActionType::BUILD_TOWER, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_fortress(MapPos pos, float priority = 0.9f) {
        return {AIActionType::BUILD_FORTRESS, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction build_gold_smelter(MapPos pos, float priority = 0.7f) {
        return {AIActionType::BUILD_GOLD_SMELTER, pos, 0, 0, 0, priority, 1.0f};
    }
    
    // Demolition Factory Methods
    static AIAction demolish_building(MapPos pos, float priority = 0.3f) {
        return {AIActionType::DEMOLISH_BUILDING, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction demolish_flag(MapPos pos, float priority = 0.3f) {
        return {AIActionType::DEMOLISH_FLAG, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction demolish_road(MapPos pos, float priority = 0.3f) {
        return {AIActionType::DEMOLISH_ROAD, pos, 0, 0, 0, priority, 1.0f};
    }
    
    static AIAction no_action() {
        return {AIActionType::NO_ACTION, 0, 0, 0, 0, 0.0f, 1.0f};
    }
    
    // Resource Priority Factory Methods
    static AIAction set_resource_priority(Resource::Type type, int priority, float action_priority = 0.4f) {
        return {static_cast<AIActionType>(static_cast<int>(AIActionType::SET_RESOURCE_PRIORITY_FISH) + static_cast<int>(type)), 
                0, 0, priority, 0, action_priority, 1.0f};
    }
    
    // Tool Priority Factory Methods  
    static AIAction set_tool_priority(int tool_index, int priority, float action_priority = 0.4f) {
        return {static_cast<AIActionType>(static_cast<int>(AIActionType::SET_TOOL_PRIORITY_SHOVEL) + tool_index), 
                0, 0, priority, 0, action_priority, 1.0f};
    }
    
    // Food Distribution Factory Methods
    static AIAction set_food_stone_mine(int amount, float action_priority = 0.5f) {
        return {AIActionType::SET_FOOD_STONE_MINE, 0, 0, amount, 0, action_priority, 1.0f};
    }
    
    static AIAction set_food_coal_mine(int amount, float action_priority = 0.5f) {
        return {AIActionType::SET_FOOD_COAL_MINE, 0, 0, amount, 0, action_priority, 1.0f};
    }
    
    static AIAction set_food_iron_mine(int amount, float action_priority = 0.5f) {
        return {AIActionType::SET_FOOD_IRON_MINE, 0, 0, amount, 0, action_priority, 1.0f};
    }
    
    static AIAction set_food_gold_mine(int amount, float action_priority = 0.5f) {
        return {AIActionType::SET_FOOD_GOLD_MINE, 0, 0, amount, 0, action_priority, 1.0f};
    }
};