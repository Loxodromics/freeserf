cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(FreeSerf)
set(PROJECT_VERSION "0.3.0")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(GitTools)
include(FetchSDL)
git_make_version(FREESERF_VERSION ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_VERSION})
message(STATUS "FREESERF_VERSION = ${FREESERF_VERSION}")

set(CMAKE_CXX_STANDARD 17)

include_directories(${INCLUDE_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(VCS_TAG "${FREESERF_VERSION}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/version-vcs.h.in
               "${CMAKE_CURRENT_BINARY_DIR}/src/version-vcs.h"
               @ONLY)

add_definitions(-DPACKAGE_BUGREPORT="https://github.com/freeserf/freeserf/issues")

include(CppLint)
enable_check_style()

# Set up SDL3 optional dependencies (mixer/image) from source
setup_sdl3_optional_dependencies()

# Find packages provided by Conan
find_package(GTest REQUIRED)

add_subdirectory(src)

option(ENABLE_TESTS "Enable compilation of tests" ON)
if(ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
