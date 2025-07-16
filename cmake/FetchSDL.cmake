# FetchSDL.cmake - Build SDL_mixer and SDL_image from GitHub for SDL3
#
# This module provides functions to fetch and build SDL_mixer and SDL_image
# from their official GitHub repositories when SDL3 packages are not available
# in the package manager. SDL3 core is provided by Conan.

include(FetchContent)

# Function to fetch and build SDL3 core from GitHub
function(fetch_sdl3)
    set(oneValueArgs TAG REPOSITORY)
    set(multiValueArgs)
    cmake_parse_arguments(FETCH_SDL3 "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Set defaults
    if(NOT FETCH_SDL3_REPOSITORY)
        set(FETCH_SDL3_REPOSITORY "https://github.com/libsdl-org/SDL.git")
    endif()
    
    if(NOT FETCH_SDL3_TAG)
        set(FETCH_SDL3_TAG "main")  # Use main branch for latest SDL3 support
    endif()
    
    message(STATUS "Fetching SDL3 from ${FETCH_SDL3_REPOSITORY} (tag: ${FETCH_SDL3_TAG})")
    
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY ${FETCH_SDL3_REPOSITORY}
        GIT_TAG ${FETCH_SDL3_TAG}
        GIT_SHALLOW TRUE
    )
    
    # Configure SDL3 build options
    set(SDL_SHARED OFF CACHE BOOL "Build SDL3 as static library")
    set(SDL_STATIC ON CACHE BOOL "Build SDL3 as static library")
    set(SDL_TEST OFF CACHE BOOL "Build SDL3 tests")
    
    FetchContent_MakeAvailable(SDL3)
endfunction()

# Function to fetch and build SDL_mixer from GitHub
function(fetch_sdl_mixer)
    set(oneValueArgs TAG REPOSITORY)
    set(multiValueArgs)
    cmake_parse_arguments(FETCH_SDL_MIXER "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Set defaults
    if(NOT FETCH_SDL_MIXER_REPOSITORY)
        set(FETCH_SDL_MIXER_REPOSITORY "https://github.com/libsdl-org/SDL_mixer.git")
    endif()
    
    if(NOT FETCH_SDL_MIXER_TAG)
        set(FETCH_SDL_MIXER_TAG "main")  # Use main branch for latest SDL3 support
    endif()
    
    message(STATUS "Fetching SDL_mixer from ${FETCH_SDL_MIXER_REPOSITORY} (tag: ${FETCH_SDL_MIXER_TAG})")
    
    FetchContent_Declare(
        SDL_mixer
        GIT_REPOSITORY ${FETCH_SDL_MIXER_REPOSITORY}
        GIT_TAG ${FETCH_SDL_MIXER_TAG}
        GIT_SHALLOW TRUE
    )
    
    # Configure SDL_mixer build options
    set(SDL3MIXER_INSTALL OFF CACHE BOOL "Install SDL3_mixer")
    set(SDL3MIXER_SAMPLES OFF CACHE BOOL "Build SDL3_mixer samples")
    set(SDL3MIXER_SHARED OFF CACHE BOOL "Build SDL3_mixer as shared library")
    
    FetchContent_MakeAvailable(SDL_mixer)
endfunction()

# Function to fetch and build SDL_image from GitHub
function(fetch_sdl_image)
    set(oneValueArgs TAG REPOSITORY)
    set(multiValueArgs)
    cmake_parse_arguments(FETCH_SDL_IMAGE "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Set defaults
    if(NOT FETCH_SDL_IMAGE_REPOSITORY)
        set(FETCH_SDL_IMAGE_REPOSITORY "https://github.com/libsdl-org/SDL_image.git")
    endif()
    
    if(NOT FETCH_SDL_IMAGE_TAG)
        set(FETCH_SDL_IMAGE_TAG "main")  # Use main branch for latest SDL3 support
    endif()
    
    message(STATUS "Fetching SDL_image from ${FETCH_SDL_IMAGE_REPOSITORY} (tag: ${FETCH_SDL_IMAGE_TAG})")
    
    FetchContent_Declare(
        SDL_image
        GIT_REPOSITORY ${FETCH_SDL_IMAGE_REPOSITORY}
        GIT_TAG ${FETCH_SDL_IMAGE_TAG}
        GIT_SHALLOW TRUE
    )
    
    # Configure SDL_image build options
    set(SDL3IMAGE_INSTALL OFF CACHE BOOL "Install SDL3_image")
    set(SDL3IMAGE_SAMPLES OFF CACHE BOOL "Build SDL3_image samples")
    
    FetchContent_MakeAvailable(SDL_image)
endfunction()

# Main function to set up SDL3 optional dependencies from source
# SDL3 core is expected to be provided by Conan
function(setup_sdl3_optional_dependencies)
    if(NOT USE_SDL3)
        message(STATUS "USE_SDL3 is OFF, skipping SDL3 optional dependencies")
        return()
    endif()
    
    message(STATUS "Setting up SDL3 optional dependencies from source (SDL3 core from Conan)")
    
    # SDL3 core should be provided by Conan via find_package
    find_package(SDL3 REQUIRED)
    
    # Create SDL3::SDL3 alias if it doesn't exist (Conan provides sdl::sdl)
    if(TARGET sdl::sdl AND NOT TARGET SDL3::SDL3)
        add_library(SDL3::SDL3 ALIAS sdl::sdl)
        message(STATUS "Created alias SDL3::SDL3 for sdl::sdl")
    endif()
    
    # Create SDL3::SDL3-shared alias for SDL3_mixer compatibility
    if(TARGET SDL3::SDL3 AND NOT TARGET SDL3::SDL3-shared)
        add_library(SDL3::SDL3-shared ALIAS SDL3::SDL3)
        message(STATUS "Created alias SDL3::SDL3-shared for SDL3::SDL3")
    endif()
    
    # Only fetch mixer/image if targets don't already exist and SDL3 is available
    if(ENABLE_SDL_MIXER AND NOT TARGET SDL3_mixer::SDL3_mixer AND TARGET SDL3::SDL3)
        fetch_sdl_mixer()
        
        # Create alias for compatibility with find_package naming
        if(TARGET SDL3_mixer AND NOT TARGET SDL3_mixer::SDL3_mixer)
            add_library(SDL3_mixer::SDL3_mixer ALIAS SDL3_mixer)
        endif()
    endif()
    
    if(ENABLE_SDL_IMAGE AND NOT TARGET SDL3_image::SDL3_image AND TARGET SDL3::SDL3)
        fetch_sdl_image()
        
        # Create alias for compatibility with find_package naming
        if(TARGET SDL3_image AND NOT TARGET SDL3_image::SDL3_image)
            add_library(SDL3_image::SDL3_image ALIAS SDL3_image)
        endif()
    endif()
endfunction()