#include "game/ecs/components/stats.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string_view>

TEST_CASE("toString converts CharacterType to readable string")
{
    REQUIRE(std::string_view{game::toString(game::CharacterType::Melee)} == "Melee");
    REQUIRE(std::string_view{game::toString(game::CharacterType::Ranged)} == "Ranged");
}

TEST_CASE("toString converts EnemyType to readable string")
{
    REQUIRE(std::string_view{game::toString(game::EnemyType::Blob)} == "Blob");
    REQUIRE(std::string_view{game::toString(game::EnemyType::Boss)} == "Boss");
}

TEST_CASE("toString returns Unknown for invalid CharacterType")
{
    const auto invalidType = static_cast<game::CharacterType>(999);

    REQUIRE(std::string_view{game::toString(invalidType)} == "Unknown");
}

TEST_CASE("toString returns Unknown for invalid EnemyType")
{
    const auto invalidType = static_cast<game::EnemyType>(999);

    REQUIRE(std::string_view{game::toString(invalidType)} == "Unknown");
}