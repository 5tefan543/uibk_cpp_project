#include "game/store/store_item.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string_view>

TEST_CASE("toString converts StoreItemType to readable string")
{
    REQUIRE(std::string_view{game::toString(game::StoreItemType::Common)} == "Common");
    REQUIRE(std::string_view{game::toString(game::StoreItemType::Uncommon)} == "Uncommon");
    REQUIRE(std::string_view{game::toString(game::StoreItemType::Rare)} == "Rare");
    REQUIRE(std::string_view{game::toString(game::StoreItemType::Epic)} == "Epic");
}