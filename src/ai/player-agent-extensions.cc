#include "player-agent-extensions.h"
#include "agent-integration.h"
#include "ai-logger.h"
#include "scripted-agent.h"
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
        
        // 2. Get actions from agent (try enhanced version first)
        std::vector<AIAction> actions;
        if (auto scripted_agent = dynamic_cast<ScriptedAgent*>(agent)) {
            // Use enhanced version with Game/Player access for ScriptedAgent
            actions = scripted_agent->get_actions(game_state, game, player);
        } else {
            // Fallback to standard version for other agents
            actions = agent->get_actions(game_state);
        }
        
        // 3. Validate and execute actions
        int actions_executed = 0;
        for (const auto& action : actions) {
            if (action.type != AIActionType::NO_ACTION) {
                // Validate action first
                auto validation = AgentIntegration::ActionValidator::validate_action(action, game, player);
                AILogger::log_action_validation(player_id, action, validation.is_valid, validation.failure_reason);
                
                if (validation.is_valid) {
                    // Use corrected position if validation provided one
                    AIAction final_action = action;
                    if (validation.corrected_position != 0) {
                        final_action.primary_position = validation.corrected_position;
                        AILogger::log_debug_info(player_id, "Using corrected position " + std::to_string(validation.corrected_position) + 
                                                 " instead of " + std::to_string(action.primary_position));
                    }
                    
                    // Execute action (potentially with corrected position)
                    std::vector<AgentIntegration::ActionResult> results = AgentIntegration::execute_actions({final_action}, game, player);
                    
                    if (!results.empty()) {
                        const auto& result = results[0];
                        float execution_time_ms = result.execution_time.count() / 1000.0f;
                        
                        AILogger::log_action_execution(player_id, action, result.success, 
                                                     result.result_message, execution_time_ms, result.reward);
                        
                        if (result.success) {
                            actions_executed++;
                        }
                    }
                } else {
                    AILogger::log_action_execution(player_id, action, false, validation.failure_reason, 0.0f, 0.0f);
                }
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