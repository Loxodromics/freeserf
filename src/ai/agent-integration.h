#pragma once

#include "agent.h"
#include "game-state.h"
#include "ai-action.h"
#include "../player.h"
#include "../game.h"
#include <vector>
#include <string>
#include <chrono>

/**
 * Error codes for AI action validation and execution
 */
enum class ActionError {
    SUCCESS = 0,
    INVALID_POSITION = 1,
    INSUFFICIENT_RESOURCES = 2,
    TERRAIN_UNSUITABLE = 3,
    POSITION_OCCUPIED = 4,
    OUT_OF_TERRITORY = 5,
    TOO_CLOSE_TO_BUILDING = 6,
    NO_ADJACENT_FLAG = 7,
    INVALID_ROAD_PATH = 8,
    GAME_ENGINE_ERROR = 9,
    UNKNOWN_ERROR = 10
};

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
    
    // Action validation
    struct ActionValidationResult {
        bool is_valid;
        std::string failure_reason;
        ActionError error_code;
        float confidence;  // 0.0-1.0, confidence in the validation result
        MapPos corrected_position = 0;  // Alternative position if original was invalid
        
        // Constructors for convenience
        ActionValidationResult(bool valid, const std::string& reason, ActionError err, float conf, MapPos corrected = 0)
            : is_valid(valid), failure_reason(reason), error_code(err), confidence(conf), corrected_position(corrected) {}
    };
    
    class ActionValidator {
    public:
        static ActionValidationResult validate_action(const AIAction& action, const Game* game, const Player* player);
        static ActionValidationResult validate_build_castle(MapPos pos, const Game* game, const Player* player);
        static ActionValidationResult validate_build_flag(MapPos pos, const Game* game, const Player* player);
        static ActionValidationResult validate_build_road(MapPos from, MapPos to, const Game* game, const Player* player);
        static ActionValidationResult validate_build_building(MapPos pos, Building::Type type, const Game* game, const Player* player);
        
    private:
        // Helper method for smart castle position finding
        static MapPos find_valid_castle_position_nearby(MapPos suggested_pos, Game* game, Player* player);
        // Helper method for smart building position finding
        static MapPos find_valid_building_position_nearby(MapPos suggested_pos, Building::Type type, Game* game, Player* player);
    };
    
    // Action execution
    struct ActionResult {
        bool success;
        float reward;
        std::string result_message;
        ActionError error_code;
        std::chrono::microseconds execution_time;
    };
    
    class ActionExecutor {
    public:
        static std::vector<ActionResult> execute_actions(const std::vector<AIAction>& actions, Game* game, Player* player);
        
    private:
        static ActionResult execute_build_castle(const AIAction& action, Game* game, Player* player);
        static ActionResult execute_build_flag(const AIAction& action, Game* game, Player* player);
        static ActionResult execute_build_road(const AIAction& action, Game* game, Player* player);
        static ActionResult execute_build_lumberjack(const AIAction& action, Game* game, Player* player);
        static ActionResult execute_build_forester(const AIAction& action, Game* game, Player* player);
        static ActionResult execute_build_building_generic(const AIAction& action, Building::Type building_type, Game* game, Player* player);
        static float calculate_building_reward(Building::Type type);
        static ActionResult create_success_result(const std::string& message, float reward, std::chrono::microseconds exec_time);
        static ActionResult create_failure_result(const std::string& message, ActionError error, std::chrono::microseconds exec_time);
    };
    
    // Legacy execute_actions method (will delegate to ActionExecutor)
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
    
    // AI player setup
    static void setup_ai_players(int ai_count);

private:
    // Helper functions for game state extraction
    static void extract_player_state(GameState::PlayerState& player_state, 
                                    const Player* player, const Game* game);
    static void extract_map_info(GameState::MapInfo& map_info, const Game* game);
    static void extract_global_info(GameState::GlobalInfo& global_info, const Game* game);
};