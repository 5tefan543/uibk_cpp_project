#include "controller/controller.hpp"
#include "controller/input/input_state.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Controller can be constructed and basic methods can be called")
{
    controller::Controller controller;
    controller::InputState input;

    REQUIRE_NOTHROW(controller.update(input, 1.0f / 60.0f));
}
