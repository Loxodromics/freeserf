cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

option(USE_SDL3 "Use SDL3 instead of SDL2" ON)
option(ENABLE_SDL_MIXER "Enable audio support using SDL_mixer" ON)  # Re-enabled for SDL3 via FetchContent
option(ENABLE_SDL_IMAGE "Enable image loading using SDL_image" OFF)  # Temporarily disabled for SDL3

if(USE_SDL3)
  add_definitions(-DUSE_SDL3)
  add_definitions(-DSDL_NO_COMPAT=1)  # Disable SDL3's compatibility layer
  if(ENABLE_SDL_MIXER)
    add_definitions(-DENABLE_SDL_MIXER)
  endif()
  if(ENABLE_SDL_IMAGE)
    add_definitions(-DENABLE_SDL_IMAGE)
  endif()
  # SDL3, SDL3_mixer and SDL3_image are built from source via FetchContent
  # Their targets are created by the FetchSDL module
else()
  find_package(SDL2 REQUIRED)
  if(ENABLE_SDL_MIXER)
    find_package(SDL2_mixer REQUIRED)
  endif()
  if(ENABLE_SDL_IMAGE)
    find_package(SDL2_image REQUIRED)
  endif()
endif()

if(WIN32)
  add_definitions(/D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS)
endif()

# Tools library

set(TOOLS_SOURCES debug.cc
                  log.cc
                  configfile.cc
                  buffer.cc)

set(TOOLS_HEADERS debug.h
                  log.h
                  misc.h
                  configfile.h
                  buffer.h)

add_library(tools STATIC ${TOOLS_SOURCES} ${TOOLS_HEADERS})
target_check_style(tools)

# Game library

set(GAME_SOURCES building.cc
                 flag.cc
                 game.cc
                 inventory.cc
                 map.cc
                 map-generator.cc
                 mission.cc
                 player.cc
                 random.cc
                 savegame.cc
                 serf.cc
                 game-manager.cc)

set(GAME_HEADERS building.h
                 flag.h
                 game.h
                 inventory.h
                 map.h
                 map-generator.h
                 map-geometry.h
                 mission.h
                 objects.h
                 player.h
                 random.h
                 resource.h
                 savegame.h
                 serf.h
                 game-manager.h)

add_library(game STATIC ${GAME_SOURCES} ${GAME_HEADERS})
target_check_style(game)

# Platform library

set(PLATFORM_SOURCES video.cc
                     audio.cc
                     event_loop.cc
                     sdl_compat.cc)

set(PLATFORM_HEADERS video.h
                     audio.h
                     event_loop.h)

list(APPEND PLATFORM_SOURCES video-sdl.cc event_loop-sdl.cc)
list(APPEND PLATFORM_HEADERS video-sdl.h event_loop-sdl.h)

if(ENABLE_SDL_MIXER)
  list(APPEND PLATFORM_SOURCES audio-sdlmixer.cc)
  list(APPEND PLATFORM_HEADERS audio-sdlmixer.h)
else()
  list(APPEND PLATFORM_SOURCES audio-dummy.cc)
  list(APPEND PLATFORM_HEADERS audio-dummy.h )
endif()

add_library(platform STATIC ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
if(USE_SDL3)
  # Check which SDL3 target is available (Conan vs system/find_package)
  if(TARGET SDL3::SDL3)
    target_link_libraries(platform SDL3::SDL3)
  elseif(TARGET sdl::sdl)
    target_link_libraries(platform sdl::sdl)
  else()
    message(FATAL_ERROR "No SDL3 target found. Expected SDL3::SDL3 or sdl::sdl")
  endif()
  
  if(ENABLE_SDL_MIXER AND TARGET SDL3_mixer::SDL3_mixer)
    target_link_libraries(platform SDL3_mixer::SDL3_mixer)
  endif()
else()
  target_link_libraries(platform SDL2::SDL2)
  if(ENABLE_SDL_MIXER)
    target_link_libraries(platform SDL2_mixer::SDL2_mixer)
  endif()
endif()
target_check_style(platform)

# Data library

set(DATA_SOURCES data.cc
                 data-source-dos.cc
                 data-source-amiga.cc
                 data-source-legacy.cc
                 data-source-custom.cc
                 tpwm.cc
                 sfx2wav.cc
                 xmi2mid.cc
                 pcm2wav.cc
                 data-source.cc)

set(DATA_HEADERS data.h
                 data-source-dos.h
                 data-source-amiga.h
                 data-source-legacy.h
                 data-source-custom.h
                 tpwm.h
                 sfx2wav.h
                 xmi2mid.h
                 pcm2wav.h
                 data-source.h
                 sprite-file.h)

if(ENABLE_SDL_IMAGE)
  list(APPEND DATA_SOURCES sprite-file-sdl.cc)
else()
  list(APPEND DATA_SOURCES sprite-file-dummy.cc)
endif()

add_library(data STATIC ${DATA_SOURCES} ${DATA_HEADERS})
if(ENABLE_SDL_IMAGE)
  if(USE_SDL3)
    target_link_libraries(data SDL3::SDL3)
    if(TARGET SDL3_image::SDL3_image)
      target_link_libraries(data SDL3_image::SDL3_image)
    endif()
  else()
    target_link_libraries(data SDL2::SDL2 SDL2_image::SDL2_image)
  endif()
endif()
target_check_style(data)

# FreeSerf executable

set(OTHER_SOURCES pathfinder.cc
                  gfx.cc
                  viewport.cc
                  minimap.cc
                  interface.cc
                  gui.cc
                  popup.cc
                  game-init.cc
                  notification.cc
                  panel.cc
                  version.cc
                  text-input.cc
                  list.cc
                  command_line.cc)

set(OTHER_HEADERS pathfinder.h
                  gfx.h
                  viewport.h
                  minimap.h
                  interface.h
                  gui.h
                  popup.h
                  game-init.h
                  notification.h
                  panel.h
                  misc.h
                  freeserf_endian.h
                  version.h
                  text-input.h
                  list.h
                  convertor.h
                  command_line.h)

set(FREESERF_SOURCES freeserf.cc ${OTHER_SOURCES})
set(FREESERF_HEADERS freeserf.h ${OTHER_HEADERS})
add_executable(FreeSerf MACOSX_BUNDLE WIN32 ${FREESERF_SOURCES} ${FREESERF_HEADERS})
target_check_style(FreeSerf)

target_link_libraries(FreeSerf game platform data tools)
if(USE_SDL3)
  # Check which SDL3 target is available (Conan vs system/find_package)
  if(TARGET SDL3::SDL3)
    target_link_libraries(FreeSerf SDL3::SDL3)
  elseif(TARGET sdl::sdl)
    target_link_libraries(FreeSerf sdl::sdl)
  else()
    message(FATAL_ERROR "No SDL3 target found. Expected SDL3::SDL3 or sdl::sdl")
  endif()
  
  if(ENABLE_SDL_MIXER AND TARGET SDL3_mixer::SDL3_mixer)
    target_link_libraries(FreeSerf SDL3_mixer::SDL3_mixer)
  endif()
  if(ENABLE_SDL_IMAGE AND TARGET SDL3_image::SDL3_image)
    target_link_libraries(FreeSerf SDL3_image::SDL3_image)
  endif()
else()
  target_link_libraries(FreeSerf SDL2::SDL2)
  if(WIN32)
    target_link_libraries(FreeSerf SDL2::SDL2main)
  endif()
  if(ENABLE_SDL_MIXER)
    target_link_libraries(FreeSerf SDL2_mixer::SDL2_mixer)
  endif()
  if(ENABLE_SDL_IMAGE)
    target_link_libraries(FreeSerf SDL2_image::SDL2_image)
  endif()
endif()
install(TARGETS FreeSerf DESTINATION ".")

if(WIN32)
  install(CODE [[
                 include(InstallRequiredSystemLibraries)
                 include(BundleUtilities)
                 fixup_bundle("${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:FreeSerf>" "" "$<TARGET_FILE_DIR:FreeSerf>")
                 verify_app("${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:FreeSerf>")
               ]])
elseif(APPLE)
  install(CODE [[
                 include(BundleUtilities)
                 set(BU_CHMOD_BUNDLE_ITEMS TRUE)
                 fixup_bundle("${CMAKE_INSTALL_PREFIX}/$<TARGET_BUNDLE_DIR_NAME:FreeSerf>" "" "")
                 verify_app("${CMAKE_INSTALL_PREFIX}/$<TARGET_BUNDLE_DIR_NAME:FreeSerf>")
               ]])
endif()

set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/COPYING)
set(CPACK_PACKAGE_VERSION_MAJOR	${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR	${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH	${PROJECT_VERSION_PATCH})
if(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(WIN_ARCH_STRING x64)
  else()
    set(WIN_ARCH_STRING x86)
  endif()
  set(CPACK_GENERATOR "NSIS")
  if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${WIN_ARCH_STRING}-Develop")
  else()
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${WIN_ARCH_STRING}-${CMAKE_BUILD_TYPE}")
  endif()
  set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
  set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
  set(CPACK_PACKAGE_EXECUTABLES "FreeSerf;FreeSerf")
  # install VC redistributables
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
    set(CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY TRUE)
  endif(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
  include(InstallRequiredSystemLibraries)
elseif(APPLE)
  set(CPACK_GENERATOR "DragNDrop")
  set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}")
else()
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}")
endif()
include(CPack)

# Profiler executable

set(PROFILER_SOURCES profiler.cc
                     version.cc
                     command_line.cc)

set(PROFILER_HEADERS profiler.h
                     version.h
                     command_line.h)

add_executable(profiler ${PROFILER_SOURCES} ${PROFILER_HEADERS})
target_check_style(profiler)
target_link_libraries(profiler game tools)
