#include "geometry/rectangle.hpp"
#include "geometry/vector.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cmath>

using geometry::Rectangle;
using geometry::Vec2;

TEST_CASE("Rectangle tests", "[rectangle-tests]")
{
    SECTION("Rectangle centered() and getCenter()")
    {
        const Vec2<int> posA = {0, 0};
        const Vec2<int> sizeA = {10, 10};
        Rectangle<int> a = {posA, sizeA};
        const Vec2<int> newCenter = {0, 0};
        const Rectangle<int> aCentered = a.centered(newCenter, sizeA);

        REQUIRE((aCentered.position == Vec2{-5, -5}).all());
        REQUIRE((aCentered.getCenter() == newCenter).all());
        REQUIRE((aCentered.size == sizeA).all());
    }

    SECTION("Rectangle centerizeX()")
    {
        const Vec2<int> posA = {0, 0};
        const Vec2<int> sizeA = {10, 10};
        Rectangle<int> a = {posA, sizeA};
        a.centerizeX(0);

        REQUIRE((a.position == Vec2{-5, 0}).all());
        REQUIRE((a.size == sizeA).all());
    }

    SECTION("Rectangle centerizeY()")
    {
        const Vec2<int> posA = {0, 0};
        const Vec2<int> sizeA = {10, 10};
        Rectangle<int> a = {posA, sizeA};
        a.centerizeY(0);

        REQUIRE((a.position == Vec2{0, -5}).all());
        REQUIRE((a.size == sizeA).all());
    }

    SECTION("Rectangle intersects()")
    {
        const Rectangle<int> a = {{-5, 5}, {10, 10}};
        Rectangle<int> b = a;
        REQUIRE(a.intersects(b));
        REQUIRE(b.intersects(a));

        b.position += {1, 1};
        b.size -= 1;
        REQUIRE(a.intersects(b));
        REQUIRE(b.intersects(a));

        auto xOffsets = {-a.size.x, a.size.x};
        auto yOffsets = {-a.size.y, a.size.y};

        b = a;
        for (const auto xOff : xOffsets) {
            for (const auto yOff : yOffsets) {
                // Corners touching
                b.position = a.position + Vec2{xOff, yOff};
                INFO(std::format("a pos. ({}, {}) size ({},{})\nb pos. ({}, {}) size ({},{})", a.position.x,
                                 a.position.y, a.size.x, a.size.y, b.position.x, b.position.y, b.size.x, b.size.y));
                REQUIRE((a.intersects(b) && b.intersects(a)));

                // Corners overlapping
                auto offTo00 = (Vec2{xOff, yOff} * -1) / 2;
                b.position += offTo00;
                REQUIRE((a.intersects(b) && b.intersects(a)));

                // b moved diagonally away from a
                b.position += offTo00 * -2;
                REQUIRE((!a.intersects(b) && !b.intersects(a)));
            }
        }
    }

    SECTION("Rectangle contains() other rectangle")
    {
        const Rectangle<int> a = {{-5, -5}, {10, 10}};

        // Same size
        Rectangle<int> b = a;
        REQUIRE(a.contains(b));
        REQUIRE(b.contains(a));

        // b completeley inside a
        b.position += {1, 1};
        b.size -= 1;
        REQUIRE(a.contains(b));
        REQUIRE(!b.contains(a));

        auto xOffsets = {-a.size.x, a.size.x};
        auto yOffsets = {-a.size.y, a.size.y};

        b = a;
        for (const auto xOff : xOffsets) {
            for (const auto yOff : yOffsets) {
                // Corners touching
                b.position = a.position + Vec2{xOff, yOff};
                INFO(std::format("a pos. ({}, {}) size ({},{})\nb pos. ({}, {}) size ({},{})", a.position.x,
                                 a.position.y, a.size.x, a.size.y, b.position.x, b.position.y, b.size.x, b.size.y));
                REQUIRE((!a.contains(b) && !b.contains(a)));

                // Corners overlapping
                auto offTo00 = (Vec2{xOff, yOff} * -1) / 2;
                b.position += offTo00;
                REQUIRE((!a.contains(b) && !b.contains(a)));

                // b moved diagonally away from a
                b.position += offTo00 * -2;
                REQUIRE((!a.contains(b) && !b.contains(a)));
            }
        }

        // Double height
        b.size = {a.size.x / 2, a.size.y * 2};
        REQUIRE((!a.contains(b) && !b.contains(a)));

        // Double width
        b.size = {a.size.x * 2, a.size.y / 2};
        REQUIRE((!a.contains(b) && !b.contains(a)));
    }

    SECTION("Rectangle contains() point")
    {
        const Rectangle<int> a = {{-5, -5}, {10, 10}};

        // Inside
        REQUIRE(a.contains(Vec2{0, 0}));

        // Corner cases
        for (auto x : {-5, 5}) {
            for (auto y : {-5, 5}) {
                REQUIRE(a.contains(Vec2{x, y}));
            }
        }

        // Outside cases
        for (auto x : {-6, 6}) {
            for (auto y : {-6, 6}) {
                REQUIRE(!a.contains(Vec2{x, y}));
            }
        }
    }
}
