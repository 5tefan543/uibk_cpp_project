# Ideally you want to require the lowest CMake version possible for your
# project. But you have to test that manually, so just go with what you are
# currently using and adjust as you go.
cmake_minimum_required(VERSION 3.28)

project(roguelike
  VERSION 0.1.0
  DESCRIPTION "An arena-survival roguelike game written in C++ using SFML"
  LANGUAGES CXX)

# This allows us to include CMake modules located in `./cmake`.
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Note that we do not set *any* global properties here!

# A dedicated function is used for setting common compile flags and properties.
include(compile_flags)

# Catch2 is used for testing.
find_package(Catch2 CONFIG REQUIRED)
# SFML is used as the multimedia library.
find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio)
# ImGui-SFML wrapper
find_package(ImGui-SFML CONFIG REQUIRED)

# Game library
add_library(roguelike_game
  src/game/game.cpp
  src/game/systems/input_system.cpp
  src/game/systems/movement_system.cpp
)

roguelike_cxx_flags(roguelike_game)
target_include_directories(roguelike_game PUBLIC include)

# Controller library
add_library(roguelike_controller
  src/controller/controller.cpp
  src/controller/state/state.cpp
  src/controller/state/state_manager.cpp
)

roguelike_cxx_flags(roguelike_controller)
target_include_directories(roguelike_controller PUBLIC include)
target_link_libraries(roguelike_controller
  PUBLIC
    roguelike_game
)

# UI library
add_library(roguelike_ui
  src/ui/ui.cpp
  src/ui/renderer.cpp
  src/ui/input_handler.cpp
  src/ui/debug_ui.cpp
)

roguelike_cxx_flags(roguelike_ui)
target_include_directories(roguelike_ui PUBLIC include)
target_link_libraries(roguelike_ui
  PUBLIC
    SFML::Graphics
    SFML::Audio
    roguelike_controller
    ImGui-SFML::ImGui-SFML
)

# Executable
add_executable(roguelike_app
  apps/main.cpp
)

roguelike_cxx_flags(roguelike_app)
set_target_properties(roguelike_app PROPERTIES OUTPUT_NAME app)
target_link_libraries(roguelike_app
  PRIVATE
    roguelike_controller
    roguelike_ui
)

add_custom_command(TARGET roguelike_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:roguelike_app>/assets
)

# Tests
# enable_testing()

# add_executable(roguelike_test_runner
#   tests/controller/controller_test.cpp
#   tests/controller/input/input_state_test.cpp
#   tests/controller/state/state_test.cpp
#   tests/controller/state/state_manager_test.cpp
#   tests/game/game_test.cpp
#   tests/game/ecs/component_storage_test.cpp
#   tests/game/ecs/registry_test.cpp
#   tests/game/ecs/systems/input_system_test.cpp
#   tests/game/ecs/systems/movement_system_test.cpp
#   tests/shared/util.cpp
# )

# roguelike_cxx_flags(roguelike_test_runner)
# target_include_directories(roguelike_test_runner PRIVATE tests)
# target_link_libraries(roguelike_test_runner
#   PRIVATE
#     roguelike_game
#     roguelike_controller
#     Catch2::Catch2WithMain
# )

# include(CTest)
# include(Catch)
# catch_discover_tests(roguelike_test_runner)