#pragma once

#include "../map.h"
#include "../building.h"
#include "../resource.h"

/**
 * AI Action Types for Phase 1
 * Will be expanded in later phases
 */
enum class AIActionType {
    // Phase 1 Actions
    BUILD_CASTLE = 0,
    BUILD_FLAG = 1,
    BUILD_ROAD = 2,
    BUILD_LUMBERJACK = 3,
    BUILD_FORESTER = 4,
    
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
    
    static AIAction no_action() {
        return {AIActionType::NO_ACTION, 0, 0, 0, 0, 0.0f, 1.0f};
    }
};