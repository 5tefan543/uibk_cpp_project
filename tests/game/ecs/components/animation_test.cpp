#include "game/ecs/components/animation.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string_view>

TEST_CASE("toString converts AnimationDirection to readable string")
{
    REQUIRE(std::string_view{game::toString(game::AnimationDirection::None)} == "None");
    REQUIRE(std::string_view{game::toString(game::AnimationDirection::Left)} == "Left");
    REQUIRE(std::string_view{game::toString(game::AnimationDirection::Right)} == "Right");
    REQUIRE(std::string_view{game::toString(game::AnimationDirection::Up)} == "Up");
    REQUIRE(std::string_view{game::toString(game::AnimationDirection::Down)} == "Down");
}

TEST_CASE("toString converts AnimationState to readable string")
{
    REQUIRE(std::string_view{game::toString(game::AnimationState::Idle)} == "Idle");
    REQUIRE(std::string_view{game::toString(game::AnimationState::Walk)} == "Walk");
    REQUIRE(std::string_view{game::toString(game::AnimationState::Attack)} == "Attack");
    REQUIRE(std::string_view{game::toString(game::AnimationState::Hit)} == "Hit");
    REQUIRE(std::string_view{game::toString(game::AnimationState::Death)} == "Death");
}

TEST_CASE("toString returns Unknown for invalid AnimationDirection")
{
    const auto invalidDirection = static_cast<game::AnimationDirection>(999);

    REQUIRE(std::string_view{game::toString(invalidDirection)} == "Unknown");
}

TEST_CASE("toString returns Unknown for invalid AnimationState")
{
    const auto invalidState = static_cast<game::AnimationState>(999);

    REQUIRE(std::string_view{game::toString(invalidState)} == "Unknown");
}