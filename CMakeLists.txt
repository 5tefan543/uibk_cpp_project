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

# Game library
add_library(roguelike_game
  src/game/game.cpp
  src/game/game_state.cpp
)

roguelike_cxx_flags(roguelike_game)
target_include_directories(roguelike_game PUBLIC include)

# Controller library
add_library(roguelike_controller
  src/controller/controller.cpp
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
)

roguelike_cxx_flags(roguelike_ui)
target_include_directories(roguelike_ui PUBLIC include)
target_link_libraries(roguelike_ui
  PUBLIC
    SFML::Graphics
    SFML::Audio
    roguelike_controller
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

# Tests
enable_testing()

add_executable(roguelike_test_runner
  tests/foo_test.cpp
)

roguelike_cxx_flags(roguelike_test_runner)
target_link_libraries(roguelike_test_runner
  PRIVATE
    roguelike_controller
    roguelike_ui
    Catch2::Catch2WithMain
)

add_test(NAME roguelike_test_runner COMMAND $<TARGET_FILE:roguelike_test_runner>)