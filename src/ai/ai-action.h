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
    
    // Special Actions
    NO_ACTION = 99,
    WAIT = 100,
    
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
    
    static AIAction no_action() {
        return {AIActionType::NO_ACTION, 0, 0, 0, 0, 0.0f, 1.0f};
    }
};