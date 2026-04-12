#include "controller/input/input_state.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("InputState has neutral default values")
{
    controller::InputState state;

    REQUIRE_FALSE(state.upPressed);
    REQUIRE_FALSE(state.downPressed);
    REQUIRE_FALSE(state.leftPressed);
    REQUIRE_FALSE(state.rightPressed);

    REQUIRE_FALSE(state.upHeld);
    REQUIRE_FALSE(state.downHeld);
    REQUIRE_FALSE(state.leftHeld);
    REQUIRE_FALSE(state.rightHeld);

    REQUIRE_FALSE(state.mouseLeftPressed);
    REQUIRE_FALSE(state.mouseRightPressed);
    REQUIRE_FALSE(state.mouseMiddlePressed);

    REQUIRE_FALSE(state.mouseLeftHeld);
    REQUIRE_FALSE(state.mouseRightHeld);
    REQUIRE_FALSE(state.mouseMiddleHeld);

    REQUIRE(state.mouseX == 0);
    REQUIRE(state.mouseY == 0);

    REQUIRE_FALSE(state.confirmPressed);
    REQUIRE_FALSE(state.cancelPressed);
}
