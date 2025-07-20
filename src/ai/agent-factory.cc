#include "agent-factory.h"
#include "test/mock-agent.h"
#include "scripted-agent.h"
#include <stdexcept>

std::unique_ptr<Agent> AgentFactory::create_agent(const AgentConfig& config) {
    switch (config.type) {
        case AgentType::SCRIPTED:
            return std::make_unique<ScriptedAgent>(config.difficulty, config.personality, config.name);
            
        case AgentType::NEURAL_NETWORK:
            // TODO: Implement NeuralAgent in future phases
            throw std::runtime_error("NeuralAgent not yet implemented");
            
        case AgentType::HUMAN_ASSISTED:
            // TODO: Implement HumanAssistedAgent in future phases
            throw std::runtime_error("HumanAssistedAgent not yet implemented");
            
        default:
            throw std::runtime_error("Unknown agent type");
    }
}

std::unique_ptr<Agent> AgentFactory::create_scripted_agent(int difficulty, 
                                                          int personality,
                                                          const std::string& name) {
    return std::make_unique<ScriptedAgent>(difficulty, personality, name);
}

std::unique_ptr<Agent> AgentFactory::create_neural_agent(const std::string& model_path,
                                                        bool training_mode,
                                                        const std::string& name) {
    // For Phase 0, return MockAgent as placeholder
    return std::make_unique<MockAgent>(name);
}

void AgentFactory::register_agent_type(AgentType type, 
                                      std::function<std::unique_ptr<Agent>(const AgentConfig&)> creator) {
    // TODO: Implement agent type registration in future phases
}

std::vector<AgentType> AgentFactory::get_available_agent_types() {
    // For Phase 0, no agent types are fully implemented
    return {};
}

std::string AgentFactory::get_agent_type_name(AgentType type) {
    switch (type) {
        case AgentType::SCRIPTED:
            return "Scripted";
        case AgentType::NEURAL_NETWORK:
            return "Neural Network";
        case AgentType::HUMAN_ASSISTED:
            return "Human Assisted";
        default:
            return "Unknown";
    }
}

bool AgentFactory::is_agent_type_available(AgentType type) {
    // For Phase 0, no agent types are fully implemented
    return false;
}