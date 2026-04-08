#include "controller/input/input_state.hpp"
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

    REQUIRE(state.mouseX == 0);
    REQUIRE(state.mouseY == 0);

    REQUIRE_FALSE(state.confirm);
    REQUIRE_FALSE(state.pause);
}
