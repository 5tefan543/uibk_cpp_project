#include "game/game.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Game can be constructed and copied")
{
    REQUIRE_NOTHROW(game::Game{});
    game::Game original;
    REQUIRE_NOTHROW(game::Game{original});
}