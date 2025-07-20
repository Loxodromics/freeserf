#pragma once

#include "ai-action.h"
#include "game-state.h"
#include "../log.h"
#include <string>

/**
 * AI Logging System for Phase 0.2
 * Provides structured logging for AI decisions and actions with clear prefixes
 */
class AILogger {
public:
    // Configuration
    static void set_debug_enabled(bool enabled);
    static bool is_debug_enabled();
    
    // Agent lifecycle logging
    static void log_agent_attached(int player_id, const std::string& agent_name);
    static void log_agent_detached(int player_id);
    
    // Action logging
    static void log_action_taken(int player_id, const AIAction& action);
    static void log_action_result(int player_id, const AIAction& action, 
                                  bool success, const std::string& reason, float reward = 0.0f);
    
    // Game state logging
    static void log_game_state_summary(const GameState& state, int player_id);
    static void log_performance_metrics(int player_id, float execution_time_ms, 
                                        int actions_executed, bool budget_exceeded);
    
    // Game lifecycle logging
    static void log_game_started(int ai_player_count);
    static void log_game_ended(bool victory, int winning_player);
    
    // Debug utilities
    static void log_debug_info(int player_id, const std::string& message);
    static void log_error(int player_id, const std::string& error_message);

private:
    static bool debug_enabled;
    
    // Helper functions
    static std::string get_player_prefix(int player_id);
    static std::string get_action_description(const AIAction& action);
    static std::string format_position(MapPos pos);
};