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