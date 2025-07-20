#pragma once

#include "agent.h"
#include "../player.h"
#include "../game.h"
#include <memory>

/**
 * Player class extensions for Agent support
 * These functions extend the existing Player class with AI capabilities
 */
namespace PlayerAgentExtensions {
    // Agent management
    void set_agent(Player* player, std::unique_ptr<Agent> agent);
    Agent* get_agent(Player* player);
    bool has_agent(const Player* player);
    
    // Agent state tracking
    void init_agent_state(Player* player);
    void cleanup_agent_state(Player* player);
    
    // Integration with Player::update()
    void update_agent_player(Player* player, Game* game, uint16_t tick_delta);
}