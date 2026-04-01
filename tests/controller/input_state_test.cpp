#include "controller/input_state.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("InputState has neutral default values")
{
    controller::InputState state;

    REQUIRE_FALSE(state.left);
    REQUIRE_FALSE(state.right);
    REQUIRE_FALSE(state.up);
    REQUIRE_FALSE(state.down);
    REQUIRE_FALSE(state.mouseLeft);
    REQUIRE_FALSE(state.mouseRight);
    REQUIRE_FALSE(state.mouseMiddle);
    REQUIRE(state.mouseX == 0.0f);
    REQUIRE(state.mouseY == 0.0f);
}
