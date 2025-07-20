# Task: Implement FreeSerf Agent API - Phase 0

## Context and Objective

You are implementing the foundational Agent API for the freeSerf game engine. This is Phase 0 of a multi-phase AI system that will eventually support scripted AI, neural networks (PPO), and other agent types. The current phase focuses on creating a clean, extensible interface without implementing complex AI logic.

FreeSerf is an open-source reimplementation of the classic city-building/RTS game "The Settlers." The Agent API will allow AI players to make decisions and execute actions within the game.

## Implementation Requirements

### Core Components to Implement

1. **Agent Interface** (`src/ai/agent.h`)
   - Pure virtual interface for all AI agents
   - Support for scripted, neural network, and human-assisted agents
   - Action space queries for ML compatibility

2. **Game State Representation** (`src/ai/game-state.h`)
   - Complete read-only snapshot of game state for AI decision-making
   - Player information, map data, resource states
   - Performance monitoring data

3. **Action System** (`src/ai/ai-action.h`)
   - Phase 1 actions: BUILD_CASTLE, BUILD_FLAG, BUILD_ROAD, BUILD_LUMBERJACK, BUILD_FORESTER
   - Compact action representation with position and metadata
   - Static factory methods for common actions

4. **Agent Factory** (`src/ai/agent-factory.h`)
   - Creation and configuration of different agent types
   - Extensible registration system for new agent types
   - Type introspection and availability queries

5. **Integration Layer** (`src/ai/agent-integration.h`)
   - Bridge between agents and game engine
   - Game state capture and action execution
   - Performance metrics and lifecycle management

6. **Player Extensions** (`src/ai/player-agent-extensions.h`)
   - Extend Player class with agent capabilities
   - Integration point for Player::update() method
   - Agent state management

7. **Test Infrastructure** (`src/ai/test/mock-agent.h`)
   - MockAgent implementation for testing
   - Controllable behavior for unit tests
   - Test utilities and helpers

### Integration Requirements

- **Minimal game engine changes**: Only modify Player::update() method
- **Backward compatibility**: Human players and existing functionality unaffected
- **Performance**: Agent processing must stay within 2-3ms budget per player per tick
- **Memory efficiency**: Avoid unnecessary allocations in game loop

### Phase 1 Action Set (Limited Scope)

Focus only on these 5 actions for Phase 1:
- `BUILD_CASTLE`: Establish headquarters and territory
- `BUILD_FLAG`: Create transportation nodes
- `BUILD_ROAD`: Connect buildings (point-to-point only)
- `BUILD_LUMBERJACK`: Wood production building
- `BUILD_FORESTER`: Tree planting building

## Technical Specifications

### Build System
- Use existing CMake build system
- Add new source files to appropriate CMakeLists.txt
- Maintain compatibility with current build process

### Code Style
- Follow existing freeSerf coding conventions
- Use existing headers and includes where possible
- Maintain consistent naming with codebase

### Dependencies
- Use only standard C++ library and existing freeSerf dependencies
- No external AI libraries at this stage
- Leverage existing game engine validation and execution methods

### Performance Requirements
- Agent update must complete within 2-3ms per player per tick
- Minimal memory overhead for agent state
- No impact on game performance when no agents are active

## Implementation Approach

### Phase 0.1: Core Interfaces (Foundation)
1. Create directory structure: `src/ai/` and `src/ai/test/`
2. Implement header files with interface definitions
3. Create basic stub implementations
4. Ensure clean compilation

### Phase 0.2: Game State Capture + Debug Features ✅ **COMPLETED**
1. ✅ Implement game state capture from real game objects
2. ✅ Add debug mode with AI logging for development/testing
3. ✅ Enable command line configuration for AI players
4. ✅ Create observable AI behavior for validation

Key deliverables: **ALL COMPLETED**

✅ Working capture_game_state() with real data - Full implementation extracting player resources, buildings, map data
✅ Command line arguments: -a (ai-debug), -p NUM (ai-players=N) - Integrated with existing CommandLine system
✅ AI logging system with prefixed output - Complete AILogger class with structured logging
✅ Game runs with AI players and visible logging - End-to-end tested and working
✅ **Auto-start functionality** - Game automatically starts with AI players, no manual intervention required

**Status**: Phase 0.2 is **COMPLETE** with all objectives met plus auto-start enhancement. Ready for Phase 0.3.

### Phase 0.3: Action Execution Framework
1. Implement action validation using existing game methods
2. Create action execution pipeline
3. Add performance monitoring
4. Test action execution with simple actions

### Phase 0.4: Player Integration
1. Modify Player class for agent support
2. Implement PlayerAgentExtensions functions
3. Add agent update call to Player::update()
4. Test integration with game loop

### Phase 0.5: Factory and Testing
1. Implement AgentFactory for agent creation
2. Create MockAgent for testing
3. Add comprehensive unit tests
4. Validate entire system integration

## Success Criteria

### Compilation Success ✅ **COMPLETED**
- [x] All source files compile without errors or warnings
- [x] CMake configuration includes new files correctly  
- [x] Builds successfully on intended platform

### Interface Completeness ✅ **COMPLETED**
- [x] All interface methods are properly declared
- [x] GameState structure contains all specified data
- [x] AIAction supports all Phase 1 action types
- [x] AgentFactory can create at least MockAgent

### Integration Verification ✅ **COMPLETED (Phase 0.2)**
- [x] Player::update() calls agent system when agent is attached
- [x] Game state capture produces valid data
- [ ] Action execution integrates with existing game validation *(Phase 0.3)*
- [x] Performance stays within 2-3ms budget (monitoring implemented)

### Testing Requirements ✅ **COMPLETED**
- [x] MockAgent implements all interface methods
- [ ] Unit tests for core functionality *(Future phase)*
- [x] Integration tests with actual game loop
- [x] Performance benchmarks for agent processing

### Functional Validation ✅ **COMPLETED (Phase 0.2)**
- [x] Can attach/detach agents to players
- [x] Game state accurately reflects current game situation
- [x] **Auto-start with AI players (no manual setup required)**
- [ ] Actions execute through existing game systems *(Phase 0.3)*
- [x] No crashes or memory leaks
- [x] Human players unaffected by agent system

## Documentation Requirements

Create comprehensive documentation in `docs/agent-api.md`:

### API Documentation
- Complete interface documentation with examples
- GameState structure field descriptions
- AIAction usage patterns and examples
- Integration guide for adding new agent types

### Implementation Guide
- Architecture overview and design decisions
- Performance considerations and measurements
- Testing strategy and available test utilities
- Troubleshooting common integration issues

### Usage Examples
- Creating and configuring agents
- Implementing custom agent types
- Game state analysis patterns
- Action execution examples

## Check-in Requirements

### Regular Progress Updates
Provide status updates every 2-3 significant commits:
- What was implemented in this session
- Current status of success criteria
- Any architectural decisions or changes made
- Issues encountered and solutions applied
- Next steps and remaining work

### Compilation Verification
After each major component implementation:
```bash
# Compile the project
mkdir -p build && cd build
cmake .. && make

# If compilation succeeds, test basic execution
timeout 10s ./src/freeserf --help  # Quick validation
```

### Testing Validation
After implementing test infrastructure:
```bash
# Run unit tests
timeout 30s ./test/agent-tests

# Run integration tests if available
timeout 60s ./test/integration-tests
```

## Deliverables

1. **Source Code**
   - All header and implementation files as specified
   - CMake configuration updates
   - Integration with existing Player class

2. **Test Suite**
   - MockAgent implementation
   - Unit tests for core components
   - Integration tests for game loop

3. **Documentation**
   - Complete API documentation
   - Implementation guide
   - Usage examples and patterns

4. **Performance Report**
   - Timing measurements for agent processing
   - Memory usage analysis
   - Performance recommendations

## Implementation Notes

- Start with interfaces and stub implementations to ensure clean architecture
- Focus on integration points early to validate approach
- Keep agent logic simple - complex AI will be added in later phases
- Prioritize clean, testable code over optimization at this stage
- Use existing freeSerf validation and execution methods where possible

Begin implementation with the core interfaces and work systematically through each component, checking in regularly with compilation and testing validation.
