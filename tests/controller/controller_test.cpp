#include "controller/controller.hpp"
#include "controller/input/input_state.hpp"
#include "shared/util.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace controller;

TEST_CASE("Controller is constructed with main menu as current state")
{
    // ARRANGE & ACT
    Controller controller;

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&controller.getCurrentState()) != nullptr);
}

TEST_CASE("Controller update with no relevant input keeps main menu")
{
    // ARRANGE
    Controller controller;
    InputState input;

    // ACT
    controller.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&controller.getCurrentState()) != nullptr);
}

TEST_CASE("Controller forwards state update result to state manager")
{
    // ARRANGE
    Controller controller;
    InputState input;
    input.confirm = true;

    // ACT
    controller.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(dynamic_cast<GameplayState *>(&controller.getCurrentState()) != nullptr);
}