#include "player-agent-extensions.h"
#include "agent-integration.h"
#include "ai-logger.h"
#include <unordered_map>
#include <chrono>

// TODO: In Phase 0.4, this will be replaced with proper Player class extension
// For now, use a global map to associate agents with players
static std::unordered_map<const Player*, std::unique_ptr<Agent>> player_agents;

namespace PlayerAgentExtensions {

void set_agent(Player* player, std::unique_ptr<Agent> agent) {
    if (agent == nullptr) {
        player_agents.erase(player);
    } else {
        player_agents[player] = std::move(agent);
    }
}

Agent* get_agent(Player* player) {
    auto it = player_agents.find(player);
    if (it != player_agents.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool has_agent(const Player* player) {
    return player_agents.find(player) != player_agents.end();
}

void init_agent_state(Player* player) {
    // TODO: Implement agent state initialization in Phase 0.4
}

void cleanup_agent_state(Player* player) {
    player_agents.erase(player);
}

void update_agent_player(Player* player, Game* game, uint16_t tick_delta) {
    Agent* agent = get_agent(player);
    if (agent == nullptr || !agent->is_ready()) {
        return;
    }
    
    // Performance timing
    auto start_time = std::chrono::high_resolution_clock::now();
    
    int player_id = player->get_index();
    
    try {
        // 1. Capture current game state
        GameState game_state = AgentIntegration::capture_game_state(game, player);
        
        // 2. Get actions from agent
        std::vector<AIAction> actions = agent->get_actions(game_state);
        
        // 3. Log and execute actions
        int actions_executed = 0;
        for (const auto& action : actions) {
            if (action.type != AIActionType::NO_ACTION) {
                AILogger::log_action_taken(player_id, action);
                
                // TODO: Implement actual action execution in Phase 0.3
                // For now, just log the action as successful
                AILogger::log_action_result(player_id, action, true, "Action logged (execution not yet implemented)", 1.0f);
                actions_executed++;
            }
        }
        
        // 4. Log periodic game state summary (every 50 ticks)
        static uint32_t last_summary_tick = 0;
        if (game_state.game_tick - last_summary_tick >= 50) {
            AILogger::log_game_state_summary(game_state, player_id);
            last_summary_tick = game_state.game_tick;
        }
        
        // 5. Performance monitoring
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        float execution_time_ms = duration.count() / 1000.0f;
        
        bool budget_exceeded = execution_time_ms > 3.0f; // 3ms budget
        AILogger::log_performance_metrics(player_id, execution_time_ms, actions_executed, budget_exceeded);
        
        if (budget_exceeded) {
            AILogger::log_debug_info(player_id, "AI execution time exceeded budget!");
        }
        
    } catch (const std::exception& e) {
        AILogger::log_error(player_id, std::string("Agent update failed: ") + e.what());
    }
}

}