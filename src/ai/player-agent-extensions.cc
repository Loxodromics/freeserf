#include "player-agent-extensions.h"
#include <unordered_map>

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
    
    // TODO: Implement full agent update cycle in Phase 0.4
    // This would include:
    // 1. Capture game state
    // 2. Get actions from agent
    // 3. Execute actions
    // 4. Provide feedback/rewards
}

}