#include "game/game.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Game can be constructed")
{
    REQUIRE_NOTHROW(game::Game{});
}