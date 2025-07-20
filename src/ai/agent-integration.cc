#include "agent-integration.h"
#include "player-agent-extensions.h"

void AgentIntegration::attach_agent(Player* player, std::unique_ptr<Agent> agent) {
    PlayerAgentExtensions::set_agent(player, std::move(agent));
}

void AgentIntegration::detach_agent(Player* player) {
    PlayerAgentExtensions::set_agent(player, nullptr);
}

Agent* AgentIntegration::get_agent(const Player* player) {
    return PlayerAgentExtensions::get_agent(const_cast<Player*>(player));
}

bool AgentIntegration::has_agent(const Player* player) {
    return PlayerAgentExtensions::has_agent(player);
}

GameState AgentIntegration::capture_game_state(const Game* game, const Player* player) {
    GameState state = {};
    
    // TODO: Implement game state capture in Phase 0.2
    // For now, return empty/default state
    state.game_tick = 0;
    state.game_speed = 1;
    
    return state;
}

void AgentIntegration::update_game_state(GameState& state, const Game* game, const Player* player) {
    // TODO: Implement game state update in Phase 0.2
}

std::vector<AgentIntegration::ActionResult> AgentIntegration::execute_actions(
    const std::vector<AIAction>& actions,
    Game* game, 
    Player* player) {
    
    std::vector<ActionResult> results;
    
    // TODO: Implement action execution in Phase 0.3
    // For now, return failed results for all actions
    for (const auto& action : actions) {
        ActionResult result;
        result.success = false;
        result.reward = 0.0f;
        result.failure_reason = "Action execution not yet implemented";
        result.error_code = -1;
        results.push_back(result);
    }
    
    return results;
}

AgentIntegration::PerformanceMetrics AgentIntegration::get_performance_metrics(const Player* player) {
    PerformanceMetrics metrics = {};
    
    // TODO: Implement performance tracking in Phase 0.3
    metrics.last_execution_time_ms = 0.0f;
    metrics.average_execution_time_ms = 0.0f;
    metrics.successful_actions = 0;
    metrics.failed_actions = 0;
    metrics.budget_exceeded = false;
    
    return metrics;
}

void AgentIntegration::reset_performance_metrics(Player* player) {
    // TODO: Implement performance tracking reset in Phase 0.3
}

void AgentIntegration::agent_player_update(Player* player, Game* game, uint16_t tick_delta) {
    PlayerAgentExtensions::update_agent_player(player, game, tick_delta);
}

void AgentIntegration::agent_game_started(const Game* game) {
    // TODO: Implement game lifecycle management in Phase 0.4
}

void AgentIntegration::agent_game_ended(const Game* game, bool victory) {
    // TODO: Implement game lifecycle management in Phase 0.4
}