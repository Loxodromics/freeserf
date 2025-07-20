#pragma once

#include "agent.h"
#include "game-state.h"
#include "ai-action.h"
#include "../player.h"
#include "../game.h"
#include <vector>
#include <string>

/**
 * Agent Integration Manager
 * Handles the integration between AI agents and the game engine
 */
class AgentIntegration {
public:
    // Player integration
    static void attach_agent(Player* player, std::unique_ptr<Agent> agent);
    static void detach_agent(Player* player);
    static Agent* get_agent(const Player* player);
    static bool has_agent(const Player* player);
    
    // Game state conversion
    static GameState capture_game_state(const Game* game, const Player* player);
    static void update_game_state(GameState& state, const Game* game, const Player* player);
    
    // Action execution
    struct ActionResult {
        bool success;
        float reward;
        std::string failure_reason;
        int error_code;
    };
    
    static std::vector<ActionResult> execute_actions(const std::vector<AIAction>& actions,
                                                   Game* game, 
                                                   Player* player);
    
    // Performance monitoring
    struct PerformanceMetrics {
        float last_execution_time_ms;
        float average_execution_time_ms;
        int successful_actions;
        int failed_actions;
        bool budget_exceeded;
    };
    
    static PerformanceMetrics get_performance_metrics(const Player* player);
    static void reset_performance_metrics(Player* player);
    
    // AI lifecycle management
    static void agent_player_update(Player* player, Game* game, uint16_t tick_delta);
    static void agent_game_started(const Game* game);
    static void agent_game_ended(const Game* game, bool victory);
};