#include "controller/controller.hpp"
#include "controller/debug/debug_context.hpp"
#include "controller/input/input_state.hpp"
#include "shared/test_fixture.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace controller;

TEST_CASE_METHOD(TestFixture, "Controller is constructed with main menu as current state")
{
    // ARRANGE & ACT
    Controller controller;

    // ASSERT
    REQUIRE(typeid(controller.getCurrentState()) == typeid(MenuState));
}

TEST_CASE_METHOD(TestFixture, "Controller update with no relevant input keeps main menu")
{
    // ARRANGE
    Controller controller;
    InputState input;

    // ACT
    controller.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(typeid(controller.getCurrentState()) == typeid(MenuState));
}

TEST_CASE_METHOD(TestFixture, "Controller forwards state update result to state manager")
{
    // ARRANGE
    Controller controller;
    InputState input;
    input.confirmPressed = true;

    // ACT
    controller.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(typeid(controller.getCurrentState()) == typeid(GameplayState));
}

TEST_CASE_METHOD(TestFixture, "Controller toggles debug mode on")
{
    // Arrange
    Controller controller;
    InputState input;
    input.toggleDebugPressed = true;

    // Act
    controller.update(input, dummyDeltaTime);

    // Assert
    REQUIRE(DebugContext::get().active == true);
}

TEST_CASE_METHOD(TestFixture, "Controller toggles debug mode off when toggle is pressed twice")
{
    // Arrange
    Controller controller;
    InputState input;
    input.toggleDebugPressed = true;

    // Act
    controller.update(input, dummyDeltaTime);
    controller.update(input, dummyDeltaTime);

    // Assert
    REQUIRE(DebugContext::get().active == false);
}