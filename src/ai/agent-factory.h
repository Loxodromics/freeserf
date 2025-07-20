#pragma once

#include "agent.h"
#include <memory>
#include <string>
#include <functional>

/**
 * Configuration for Agent creation
 */
struct AgentConfig {
    AgentType type;
    int difficulty;      // 0-10
    int personality;     // 0-10  
    std::string name;
    
    // Type-specific configuration
    struct ScriptedConfig {
        bool aggressive;
        bool economic_focus;
        float decision_delay;  // Artificial thinking time
    } scripted;
    
    struct NeuralConfig {
        std::string model_path;
        bool training_mode;
        float exploration_rate;
    } neural;
};

/**
 * Factory for creating different types of AI agents
 */
class AgentFactory {
public:
    // Main factory method
    static std::unique_ptr<Agent> create_agent(const AgentConfig& config);
    
    // Convenience methods for common configurations
    static std::unique_ptr<Agent> create_scripted_agent(int difficulty = 5, 
                                                       int personality = 0,
                                                       const std::string& name = "ScriptedAgent");
    
    static std::unique_ptr<Agent> create_neural_agent(const std::string& model_path,
                                                     bool training_mode = false,
                                                     const std::string& name = "NeuralAgent");
    
    // Agent type registration (for extensibility)
    static void register_agent_type(AgentType type, 
                                   std::function<std::unique_ptr<Agent>(const AgentConfig&)> creator);
    
    // Information queries
    static std::vector<AgentType> get_available_agent_types();
    static std::string get_agent_type_name(AgentType type);
    static bool is_agent_type_available(AgentType type);
};