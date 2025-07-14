# Color Channel Debugging Guide for FreeSerf

This document explains how to use the color channel debugging features implemented to troubleshoot color issues in the SDL2 to SDL3 migration.

## Overview

During the SDL2 to SDL3 migration, color channels may appear swapped or incorrect due to pixel format differences between SDL versions. This debugging system helps identify and fix these issues.

## Root Cause of Color Issues

The color channel swapping occurs due to inconsistencies in the asset loading pipeline:

1. **Pixel Format Mismatches**: Different parts of the code use different pixel formats (ARGB8888 vs RGBA8888)
2. **Hardcoded Color Masks**: Fixed color masks that don't match the actual pixel format
3. **Data Source Inconsistencies**: Original asset data format (BGRA) not consistently handled
4. **SDL Version Differences**: SDL2 and SDL3 handle pixel formats differently

## Command Line Options

### Basic Usage

```bash
# Show all available options including color debugging
./FreeSerf -h

# Enable basic debug logging
./FreeSerf -d 1

# Enable color channel debugging (requires DEBUG build)
./FreeSerf -d 1 -c

# Enable color channel debugging with R/B channel swapping
./FreeSerf -d 1 -c -s

# Most verbose logging with all color debugging features
./FreeSerf -d 0 -c -s
```

### Debug Level Options

- `-d 0`: Verbose level (most detailed)
- `-d 1`: Debug level 
- `-d 2`: Info level (default)
- `-d 3`: Warning level
- `-d 4`: Error level only

### Color Debug Options

- `-c`: Enable color channel debugging and pixel format validation
- `-s`: Swap R/B color channels as a debugging test

## Build Requirements

The color debugging features are only available in **DEBUG builds**. To ensure you have a debug build:

```bash
# Create debug build
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
ninja

# Or with Conan 2
mkdir build
conan install . --output-folder=build --build=missing
cd build
cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
ninja
```

## Expected Debug Output

### Successful Activation

When the debugging flags are properly activated, you'll see:

```
Info: [main] freeserf 0.3.3ce6c12
Info: [main] Color channel debugging enabled
Info: [main] R/B channel swapping enabled
```

### Sprite Loading Debug Information

When sprites are loaded and processed, you'll see detailed information:

```
Debug: [video] create_surface_from_data: Creating surface 32x32
Debug: [video]   Pixel format: SDL_PIXELFORMAT_RGBA8888
Debug: [video]   Masks - R: 0xff000000 G: 0x00ff0000 B: 0x0000ff00 A: 0x000000ff
Debug: [video]   Input surface format: SDL_PIXELFORMAT_RGBA8888
Debug: [video]   Converted surface format: SDL_PIXELFORMAT_RGBA8888
Debug: [video] debug_validate_and_swap_channels: Processing surface 32x32
Debug: [video] First 4 pixels (raw):
Debug: [video]   Pixel 0: 0x12345678 R=18 G=52 B=86 A=120
Debug: [video]   Pixel 1: 0x87654321 R=135 G=101 B=67 A=33
Debug: [video]   Pixel 2: 0xaabbccdd R=170 G=187 B=204 A=221
Debug: [video]   Pixel 3: 0xffeeddcc R=255 G=238 B=221 A=204
```

### Channel Swapping Output

When `-s` flag is used, you'll also see:

```
Debug: [video] debug_validate_and_swap_channels: Swapping R and B channels
```

## Interpreting the Debug Output

### 1. Pixel Format Information

- **Pixel format**: Shows the SDL pixel format being used (e.g., SDL_PIXELFORMAT_RGBA8888)
- **Masks**: Shows the bit masks for each color component
- **Input/Converted surface format**: Shows format changes during processing

### 2. Pixel Data Analysis

- **Raw pixel values**: Hexadecimal representation of pixel data
- **RGBA components**: Decoded red, green, blue, and alpha values
- **Channel order verification**: Helps identify if channels are in the expected order

### 3. Common Issues to Look For

#### Pixel Format Mismatches
```
Debug: [video]   Input surface format: SDL_PIXELFORMAT_ARGB8888
Debug: [video]   Converted surface format: SDL_PIXELFORMAT_RGBA8888
```
This indicates a format conversion that might cause color issues.

#### Incorrect Color Masks
```
Debug: [video]   Masks - R: 0x00ff0000 G: 0x0000ff00 B: 0x000000ff A: 0xff000000
```
If masks don't match the pixel format, colors will be interpreted incorrectly.

#### Channel Order Problems
```
Debug: [video]   Pixel 0: 0xff0000ff R=255 G=0 B=0 A=255
```
A red pixel should show high R value, low G/B values. If values are swapped, channels are incorrect.

## Troubleshooting Steps

### Step 1: Verify Debug Build
```bash
# Check that you have a debug build
grep -i "CMAKE_BUILD_TYPE.*Debug" build/CMakeCache.txt
```

### Step 2: Basic Color Debugging
```bash
# Start with basic color debugging
./FreeSerf -d 1 -c
```

### Step 3: Test Channel Swapping
If colors appear wrong, test with channel swapping:
```bash
# Test if R/B swap fixes the issue
./FreeSerf -d 1 -c -s
```

### Step 4: Analyze Output
1. **Check pixel formats**: Ensure consistent formats throughout the pipeline
2. **Verify color masks**: Ensure masks match the pixel format
3. **Examine pixel values**: Look for patterns in color channel data
4. **Test channel swapping**: See if swapping fixes visual issues

## Common Color Issues and Solutions

### Issue 1: Red and Blue Channels Swapped
**Symptoms**: Red objects appear blue, blue objects appear red
**Debug command**: `./FreeSerf -d 1 -c -s`
**Solution**: If swapping fixes it, the issue is RGBA vs BGRA ordering

### Issue 2: All Colors Appear Wrong
**Symptoms**: Multiple color channels affected
**Debug command**: `./FreeSerf -d 1 -c`
**Solution**: Check for pixel format mismatches in the debug output

### Issue 3: Transparency Issues
**Symptoms**: Sprites have incorrect transparency
**Debug command**: `./FreeSerf -d 1 -c`
**Solution**: Check alpha channel values in pixel data

## Technical Implementation Details

### Files Modified
- `src/freeserf.cc`: Command line parameter handling
- `src/video-sdl.cc`: Debug functions and color processing
- `src/video-sdl.h`: Function declarations
- `src/sprite-file-sdl.cc`: Pixel format consistency fixes

### Key Functions
- `set_debug_color_channels(bool)`: Enables color debugging
- `set_swap_rb_channels(bool)`: Enables R/B channel swapping
- `debug_validate_and_swap_channels()`: Performs pixel analysis

### Global Debug Flags
- `global_debug_color_channels`: Controls debug output
- `global_swap_rb_channels`: Controls channel swapping

## Limitations

1. **Debug builds only**: Color debugging requires DEBUG build configuration
2. **Sprite loading timing**: Debug output only appears when sprites are actively loaded
3. **Performance impact**: Debug mode may affect game performance
4. **Console output**: Debug information goes to console/terminal only

## Example Debugging Session

```bash
# 1. Start with color debugging enabled
./FreeSerf -d 1 -c

# Look for output like:
# Info: [main] Color channel debugging enabled
# Debug: [video] create_surface_from_data: Creating surface 32x32

# 2. If colors appear wrong, test channel swapping
./FreeSerf -d 1 -c -s

# Look for:
# Info: [main] R/B channel swapping enabled
# Debug: [video] debug_validate_and_swap_channels: Swapping R and B channels

# 3. Compare visual output with and without swapping
# If swapping fixes colors, implement permanent fix in code
```

## Getting Help

If you encounter issues with the color debugging:

1. Ensure you have a DEBUG build (`CMAKE_BUILD_TYPE=Debug`)
2. Check that the flags are recognized in the help output (`./FreeSerf -h`)
3. Verify the activation messages appear in the log output
4. Report issues at: https://github.com/freeserf/freeserf/issues

## Related Files

- `CLAUDE.md`: General build and development instructions
- `SDL3-migration.md`: SDL3 migration documentation
- `src/sdl_compat.h`: SDL2/SDL3 compatibility layer
- `src/video-sdl.cc`: Main video system implementation