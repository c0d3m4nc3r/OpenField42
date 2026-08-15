cmake_minimum_required(VERSION 3.27)

project(OpenField42 VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# --- SDL3 ---
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG main
)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)

message(STATUS "Fetching SDL3...")

FetchContent_MakeAvailable(SDL3)

# --- SDL3 end ---

# --- glm ---
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        master
)

message(STATUS "Fetching glm...")

FetchContent_MakeAvailable(glm)

# --- glm end ---

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(SOURCES
    ${SRC_DIR}/core/console.cpp
    ${SRC_DIR}/core/debugui.cpp
    ${SRC_DIR}/core/engine.cpp
    ${SRC_DIR}/game/game.cpp
    ${SRC_DIR}/geometry/geometry_manager.cpp
    ${SRC_DIR}/geometry/geometry.cpp
    ${SRC_DIR}/geometry/material.cpp
    ${SRC_DIR}/geometry/standard_mesh.cpp
    ${SRC_DIR}/geometry/tree_mesh.cpp
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/math/aabb.cpp
    ${SRC_DIR}/math/frustum.cpp
    ${SRC_DIR}/object/object.cpp
    ${SRC_DIR}/object/object_template.cpp
    ${SRC_DIR}/platform/input.cpp
    ${SRC_DIR}/platform/window.cpp
    ${SRC_DIR}/render/passes/sky_pass.cpp
    ${SRC_DIR}/render/passes/std_op_pass.cpp
    ${SRC_DIR}/render/passes/std_tr_pass.cpp
    ${SRC_DIR}/render/passes/terrain_pass.cpp
    ${SRC_DIR}/render/passes/tree_op_pass.cpp
    ${SRC_DIR}/render/passes/tree_tr_pass.cpp
    ${SRC_DIR}/render/passes/water_pass.cpp
    ${SRC_DIR}/render/camera.cpp
    ${SRC_DIR}/render/renderer.cpp
    ${SRC_DIR}/render/render_pass.cpp
    ${SRC_DIR}/render/shader_manager.cpp
    ${SRC_DIR}/render/shader.cpp
    ${SRC_DIR}/render/texture_manager.cpp
    ${SRC_DIR}/render/texture.cpp
    ${SRC_DIR}/utils/gl_utils.cpp
    ${SRC_DIR}/utils/log.cpp
    ${SRC_DIR}/utils/string_utils.cpp
    ${SRC_DIR}/utils/texture_utils.cpp
    ${SRC_DIR}/vfs/folder_provider.cpp
    ${SRC_DIR}/vfs/rfa_provider.cpp
    ${SRC_DIR}/vfs/vfs.cpp
    ${SRC_DIR}/world/sky.cpp
    ${SRC_DIR}/world/terrain.cpp
    ${SRC_DIR}/world/water.cpp
    ${SRC_DIR}/world/world.cpp
)

add_executable(of42 ${SOURCES})

if (MSVC)
    target_compile_definitions(of42 PRIVATE "_CRT_SECURE_NO_WARNINGS")
endif()

target_include_directories(of42 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(of42 PUBLIC SDL3::SDL3-static glad glm librfa soil2 imgui)

target_precompile_headers(of42 PRIVATE
    <vector>
    <string>
    <memory>
    <SDL3/SDL.h>
    <glm/glm.hpp>
    <glm/gtc/matrix_transform.hpp>
    "vendor/glad/include/glad/gl.h"
    "${SRC_DIR}/utils/log.h"
)
