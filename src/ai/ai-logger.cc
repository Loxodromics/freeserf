#include "ai-logger.h"
#include <sstream>
#include <iomanip>

// Static member initialization
bool AILogger::debug_enabled = false;

void AILogger::set_debug_enabled(bool enabled) {
    debug_enabled = enabled;
    if (enabled) {
        Log::Info["ai"] << "AI debug logging enabled";
    }
}

bool AILogger::is_debug_enabled() {
    return debug_enabled;
}

void AILogger::log_agent_attached(int player_id, const std::string& agent_name) {
    if (!debug_enabled) return;
    
    Log::Info["ai"] << "[AI-ATTACH] " << get_player_prefix(player_id) 
                    << ": " << agent_name << " attached";
}

void AILogger::log_agent_detached(int player_id) {
    if (!debug_enabled) return;
    
    Log::Info["ai"] << "[AI-DETACH] " << get_player_prefix(player_id) 
                    << ": agent detached";
}

void AILogger::log_action_taken(int player_id, const AIAction& action) {
    if (!debug_enabled) return;
    
    Log::Info["ai"] << "[AI-ACTION] " << get_player_prefix(player_id) 
                    << ": " << get_action_description(action)
                    << " priority=" << std::fixed << std::setprecision(1) << action.priority
                    << " confidence=" << std::fixed << std::setprecision(1) << action.confidence;
}

void AILogger::log_action_result(int player_id, const AIAction& action, 
                                 bool success, const std::string& reason, float reward) {
    if (!debug_enabled) return;
    
    std::string result = success ? "SUCCESS" : "FAILED";
    std::string reward_str = "";
    if (success && reward != 0.0f) {
        std::ostringstream oss;
        oss << " (reward: " << std::showpos << std::fixed << std::setprecision(1) << reward << ")";
        reward_str = oss.str();
    }
    
    Log::Info["ai"] << "[AI-RESULT] " << get_player_prefix(player_id) 
                    << ": " << get_action_description(action) << " -> " << result;
    
    if (!success || (success && reward != 0.0f)) {
        Log::Info["ai"] << "    " << reason << reward_str;
    }
}

// Phase 0.3: Enhanced action logging methods
void AILogger::log_action_validation(int player_id, const AIAction& action, 
                                    bool is_valid, const std::string& reason) {
    if (!debug_enabled) return;
    
    std::string result = is_valid ? "VALID" : "INVALID";
    
    Log::Info["ai"] << "[AI-VALIDATE] " << get_player_prefix(player_id) 
                    << ": " << get_action_description(action) << " -> " << result;
    
    if (!is_valid || !reason.empty()) {
        Log::Info["ai"] << "    " << reason;
    }
}

void AILogger::log_action_execution(int player_id, const AIAction& action, 
                                   bool success, const std::string& message, 
                                   float execution_time_ms, float reward) {
    if (!debug_enabled) return;
    
    std::string result = success ? "SUCCESS" : "FAILED";
    
    Log::Info["ai"] << "[AI-EXECUTE] " << get_player_prefix(player_id) 
                    << ": " << get_action_description(action) << " -> " << result
                    << " (execution: " << std::fixed << std::setprecision(1) << execution_time_ms << "ms";
    
    if (success && reward != 0.0f) {
        Log::Info["ai"] << ", reward: " << std::showpos << std::fixed << std::setprecision(1) << reward;
    }
    
    Log::Info["ai"] << ")";
    
    if (!message.empty()) {
        Log::Info["ai"] << "    " << message;
    }
}

void AILogger::log_agent_state_change(int player_id, const std::string& old_state, 
                                     const std::string& new_state) {
    if (!debug_enabled) return;
    
    Log::Info["ai"] << "[AI-STATE] " << get_player_prefix(player_id) 
                    << ": Agent state: " << old_state << " -> " << new_state;
}

void AILogger::log_reward_calculation(int player_id, float total_reward, 
                                     const std::string& breakdown) {
    if (!debug_enabled) return;
    
    Log::Info["ai"] << "[AI-REWARD] " << get_player_prefix(player_id) 
                    << ": Total reward: " << std::showpos << std::fixed << std::setprecision(1) << total_reward;
    
    if (!breakdown.empty()) {
        Log::Info["ai"] << " (" << breakdown << ")";
    }
}

void AILogger::log_game_state_summary(const GameState& state, int player_id) {
    if (!debug_enabled) return;
    
    const auto& player_state = state.self;
    
    // Count key resources
    int logs = player_state.resource_counts[6];  // TypeLumber = 6
    int stones = player_state.resource_counts[9]; // TypeStone = 9
    int food = player_state.resource_counts[0] + player_state.resource_counts[1] + 
               player_state.resource_counts[2] + player_state.resource_counts[5]; // Fish, Pig, Meat, Bread
    
    // Count buildings
    int buildings = 0;
    for (int count : player_state.building_counts) {
        buildings += count;
    }
    
    Log::Info["ai"] << "[AI-STATE] Tick " << state.game_tick << ": " 
                    << get_player_prefix(player_id) 
                    << " - Logs:" << logs << ", Stones:" << stones << ", Food:" << food
                    << ", Buildings:" << buildings << ", Territory:" << player_state.territory_size << "tiles"
                    << ", Knights:" << player_state.castle_knights << ", Serfs:" << player_state.total_serfs;
}

void AILogger::log_performance_metrics(int player_id, float execution_time_ms, 
                                       int actions_executed, bool budget_exceeded) {
    if (!debug_enabled) return;
    
    std::string warning = budget_exceeded ? " [BUDGET EXCEEDED!]" : "";
    
    Log::Info["ai"] << "[AI-PERF] " << get_player_prefix(player_id) 
                    << ": " << std::fixed << std::setprecision(2) << execution_time_ms << "ms"
                    << ", " << actions_executed << " actions" << warning;
}

void AILogger::log_game_started(int ai_player_count) {
    if (!debug_enabled) return;
    
    Log::Info["ai"] << "[AI-DEBUG] Game started with " << ai_player_count << " AI players";
}

void AILogger::log_game_ended(bool victory, int winning_player) {
    if (!debug_enabled) return;
    
    if (victory) {
        Log::Info["ai"] << "[AI-DEBUG] Game ended - Player" << winning_player << " won";
    } else {
        Log::Info["ai"] << "[AI-DEBUG] Game ended - no winner";
    }
}

void AILogger::log_debug_info(int player_id, const std::string& message) {
    if (!debug_enabled) return;
    
    Log::Debug["ai"] << "[AI-DEBUG] " << get_player_prefix(player_id) << ": " << message;
}

void AILogger::log_debug(const std::string& message) {
    if (!debug_enabled) return;
    
    Log::Debug["ai"] << "[AI-DEBUG] " << message;
}

void AILogger::log_error(int player_id, const std::string& error_message) {
    // Always log errors, regardless of debug setting
    Log::Error["ai"] << "[AI-ERROR] " << get_player_prefix(player_id) << ": " << error_message;
}

// Helper functions
std::string AILogger::get_player_prefix(int player_id) {
    return "Player" + std::to_string(player_id);
}

std::string AILogger::get_action_description(const AIAction& action) {
    std::ostringstream oss;
    
    switch (action.type) {
        case AIActionType::BUILD_CASTLE:
            oss << "BUILD_CASTLE(" << format_position(action.primary_position) << ")";
            break;
        case AIActionType::BUILD_FLAG:
            oss << "BUILD_FLAG(" << format_position(action.primary_position) << ")";
            break;
        case AIActionType::BUILD_ROAD:
            oss << "BUILD_ROAD(" << format_position(action.primary_position) 
                << " -> " << format_position(action.secondary_position) << ")";
            break;
        case AIActionType::BUILD_LUMBERJACK:
            oss << "BUILD_LUMBERJACK(" << format_position(action.primary_position) << ")";
            break;
        case AIActionType::BUILD_FORESTER:
            oss << "BUILD_FORESTER(" << format_position(action.primary_position) << ")";
            break;
        case AIActionType::NO_ACTION:
            oss << "NO_ACTION";
            break;
        case AIActionType::WAIT:
            oss << "WAIT";
            break;
        default:
            oss << "UNKNOWN_ACTION(" << static_cast<int>(action.type) << ")";
            break;
    }
    
    return oss.str();
}

std::string AILogger::format_position(MapPos pos) {
    // MapPos is typically encoded as a single value
    // For now, just display the raw position value
    // TODO: Convert to proper x,y coordinates using map geometry
    return std::to_string(pos);
}