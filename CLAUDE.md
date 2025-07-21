# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

FreeSerf is a C++ implementation of the classic strategy game Settlers, using CMake as the build system. The project is structured with multiple static libraries and follows the Google C++ Style Guide.

## Workflow
FreeSerf AI Agent API development is tracked in work-logs. Phase 0.3 (Action Execution Framework) is now COMPLETE ✅. 

**Current Status**: Functional AI agents can play FreeSerf with complete observe-decide-act cycles.

## Build Commands

### Traditional Build (current working approach)
```bash
# Build project
mkdir build && cd build
cmake -G Ninja ..
ninja

# Run tests
ninja test

# Check code style
ninja check_style
```

### Conan 2 Build (modern dependency management)
```bash
# Create build directory and install Conan 2 dependencies
mkdir build
conan install . --output-folder=build --build=missing

# Build project with Conan 2
cd build
cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
ninja

# Run tests
ninja test

# Check code style
ninja check_style
```

### SDL3 Build (default)
```bash
# Traditional Build with SDL3 (default)
mkdir build && cd build
cmake -G Ninja -DUSE_SDL3=ON -DENABLE_SDL_MIXER=ON ..
ninja

# Conan 2 Build with SDL3 (recommended)
mkdir build
conan install . --output-folder=build --build=missing
cd build
cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_SDL3=ON -DENABLE_SDL_MIXER=ON
ninja

# Run tests
ninja test

# Check code style
ninja check_style
```

### SDL2 Build (legacy)
```bash
# Traditional Build with SDL2
mkdir build && cd build
cmake -G Ninja -DUSE_SDL3=OFF ..
ninja

# Conan 2 Build with SDL2 (when needed)
mkdir build
conan install . --output-folder=build --build=missing -o use_sdl3=False
cd build
cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_SDL3=OFF
ninja

# Run tests
ninja test

# Check code style
ninja check_style
```

## Code Architecture

### Core Libraries

- **tools**: Utility functions (debug, log, configfile, buffer)
- **game**: Core game logic (buildings, flags, serfs, map, players, AI)
- **platform**: Platform abstraction (video, audio, event handling via SDL2/SDL3)
- **data**: Resource management (DOS/Amiga data files, sprites, audio conversion)

### Main Components

- **Game class** (`src/game.h`): Central game state manager handling players, map, buildings, serfs, and game updates
- **Map** (`src/map.h`): Game world terrain and positioning
- **Player** (`src/player.h`): Player state and AI
- **Building/Flag/Serf** (`src/building.h`, `src/flag.h`, `src/serf.h`): Core game entities
- **Interface** (`src/interface.h`): Game UI and user interaction

### Key Architecture Patterns

- Uses template-based Collection classes for efficient object management
- Entity-component pattern for game objects (buildings, serfs, flags)
- Shared pointers for memory management (PGame, PMap typedefs)
- Observer pattern for game state updates

## Dependencies

### Traditional Build
- SDL3 (default) or SDL2 (legacy support)
- SDL3_mixer/SDL2_mixer (SDL3_mixer built from source via FetchContent, optional for audio)
- SDL3_image/SDL2_image (temporarily disabled for SDL3, optional for custom resources)
- GoogleTest (automatically downloaded for tests)

### Conan 2 Build
- Conan 2.0+ package manager
- Hybrid dependency approach:
  - SDL3: sdl/3.2.14 from Conan (default)
  - SDL3_mixer: Built from source via FetchContent (not available in Conan yet)
  - SDL2: sdl/2.28.3, sdl_mixer/2.8.0, sdl_image/2.8.2 (legacy)
  - gtest/1.14.0 (for tests)

### SDL3 Migration Status
- **Core SDL3**: ✅ Complete - Video, events, rendering, timers
- **SDL3_mixer**: ✅ Complete - Audio fully working via FetchContent build
- **SDL3_image**: ⏳ Temporarily disabled - Will be re-enabled when stable packages available
- **Compatibility Layer**: ✅ Complete - `src/sdl_compat.h` provides SDL2/SDL3 abstraction with full audio support

## Testing

Uses GoogleTest framework with three test suites:
- `test_map`: Map functionality tests
- `test_save_game`: Save/load game tests  
- `test_map_geometry`: Map geometry tests

## Style Guide

Follows Google C++ Style Guide. Style checking is integrated into the build system using cpplint.

## Game Data

Requires original game data files:
- DOS: `SPAE.PA`, `SPAD.PA`, `SPAF.PA`, or `SPAU.PA`
- Amiga: `gfxheader`, `gfxfast`, `gfxchip`, `gfxpics`, `sounds`, `music`

Place data files in same directory as executable or `~/.local/share/freeserf/`

## AI Agent System

### AI Testing Commands
```bash
# Test with 1 AI player (note: Player0 is reserved for human control)
./FreeSerf -a -p 2

# Test with 2 AI players
./FreeSerf -a -p 3

# Test with 3 AI players  
./FreeSerf -a -p 4

# Enable AI debug logging
./FreeSerf -a -p 2   # -a enables debug logging, -p 2 creates 1 functional AI player
```

### AI Player Indexing
**Important**: Player0 is hardcoded as the human player in FreeSerf's UI system.
- `-p 1` = 0 functional AI players (Player0 is human-controlled)
- `-p 2` = 1 functional AI player (Player1)
- `-p 3` = 2 functional AI players (Player1, Player2)
- `-p 4` = 3 functional AI players (Player1, Player2, Player3)

### AI System Status
- **Phase 0.3 COMPLETE** ✅: Action Execution Framework & Building Sequence
  - Action validation using authoritative game APIs (Game::can_build_castle, can_build_building, can_build_flag)
  - Action execution with sub-millisecond performance monitoring
  - ScriptedAgent with state machine decision-making (NEED_CASTLE → NEED_FORESTER → NEED_LUMBERJACK → NEED_ROADS → PRODUCING → EXPANDING)
  - Complete observe-decide-act cycle operational with building sequence: BUILD_CASTLE + BUILD_FORESTER + BUILD_LUMBERJACK + BUILD_FLAG
  - Building detection system counting both completed and incomplete buildings (fixes construction site issue)
  - Enhanced debug logging for full AI observability and performance tracking
  - Multi-player AI coordination verified (multiple AI players building simultaneously)
  - Building limits implemented (no infinite forester loops)

### AI Architecture
- **Agent Integration** (`src/ai/agent-integration.h/.cc`): Core AI system interface
- **ScriptedAgent** (`src/ai/scripted-agent.h/.cc`): Functional AI player implementation
- **Action System** (`src/ai/ai-action.h`): AI action definitions and validation
- **Logging System** (`src/ai/ai-logger.h/.cc`): Comprehensive AI debug logging
- **Player Extensions** (`src/ai/player-agent-extensions.h/.cc`): Player-AI integration