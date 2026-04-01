#include "game/game_state.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("GameState can be constructed")
{
    REQUIRE_NOTHROW(game::GameState{});
}
