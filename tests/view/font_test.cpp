#include "view/font.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string_view>

TEST_CASE("toString converts FontType to readable string")
{
    REQUIRE(std::string_view{view::toString(view::FontType::Default)} == "Default");
}

TEST_CASE("toString returns Unknown for invalid FontType")
{
    const auto invalidFontType = static_cast<view::FontType>(999);

    REQUIRE(std::string_view{view::toString(invalidFontType)} == "Unknown");
}