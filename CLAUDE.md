# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

FreeSerf is a C++ implementation of the classic strategy game Settlers, using CMake as the build system. The project is structured with multiple static libraries and follows the Google C++ Style Guide.

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

## Code Architecture

### Core Libraries

- **tools**: Utility functions (debug, log, configfile, buffer)
- **game**: Core game logic (buildings, flags, serfs, map, players, AI)
- **platform**: Platform abstraction (video, audio, event handling via SDL2)
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
- SDL2 (required)
- SDL2_mixer (optional, for audio)
- SDL2_image (optional, for custom resources)
- GoogleTest (automatically downloaded for tests)

### Conan 2 Build
- Conan 2.0+ package manager
- All dependencies managed through `conanfile.py`:
  - sdl/2.28.3
  - sdl_mixer/2.8.0 (optional)
  - sdl_image/2.8.2 (optional)
  - gtest/1.14.0 (for tests)

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