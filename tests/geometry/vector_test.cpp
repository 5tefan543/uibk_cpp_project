#include "geometry/vector.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cmath>

TEST_CASE("Vector equality")
{
    Vec2<int> a = {24, 24};
    Vec2<int> b = {42, 42};
    REQUIRE(a + b == Vec2<int>(66, 66));
}

TEST_CASE("Vector addition")
{
    Vec2<int> a = {1, 2};
    Vec2<int> b = {3, 4};
    REQUIRE(a + b == Vec2<int>(4, 6));
    REQUIRE(a + 42 == Vec2<int>(43, 44));
    a += b;
    REQUIRE(a == Vec2<int>{4, 6});
    b += 24;
    REQUIRE(b == Vec2<int>{27, 28});
}

TEST_CASE("Vector subtraction")
{
    Vec2<int> a = {1, 2};
    Vec2<int> b = {4, 3};
    REQUIRE(a - b == Vec2<int>(-3, -1));
    REQUIRE(a - 1 == Vec2<int>(0, 1));
    a -= b;
    REQUIRE(a == Vec2<int>(-3, -1));
    b -= 1;
    REQUIRE(b == Vec2<int>{3, 2});
}

TEST_CASE("Vector multiplication")
{
    Vec2<int> a = {1, 2};
    Vec2<int> b = {3, 4};
    REQUIRE(a * b == Vec2<int>(3, 8));
    REQUIRE(a * 2 == Vec2<int>(2, 4));
    a *= b;
    REQUIRE(a == Vec2<int>(3, 8));
    b *= 2;
    REQUIRE(b == Vec2<int>{6, 8});
}

TEST_CASE("Vector division")
{
    Vec2<int> a = {42, 24};
    Vec2<int> b = {7, 2};
    REQUIRE(a / b == Vec2<int>(6, 12));
    REQUIRE(a / 2 == Vec2<int>(21, 12));
    a /= b;
    REQUIRE(a == Vec2<int>(6, 12));
    b /= 2;
    REQUIRE(b == Vec2<int>{3, 1});
}

TEST_CASE("Vector length and normalization")
{
    REQUIRE(Vec2<int>{42, 0}.length() == 42.0f);
    REQUIRE(Vec2<int>{0, 42}.length() == 42.0f);
    REQUIRE(Vec2<int>{42, 42}.length() - std::sqrt(static_cast<float>(2 * 42 * 42)) < 0.001);
    Vec2<float> a = {42, 42};
    auto aLen = a.length();
    Vec2<float> b = a;
    b.normalize();
    REQUIRE(std::abs((a / aLen).length() - b.length()) < 0.001);
    b.setLenght(aLen);
    REQUIRE(std::abs(a.length() - b.length()) < 0.001);
}

TEST_CASE("Vectors methods")
{
    const Vec2<int> a = {42, -42};
    const Vec2<int> b = {-42, 42};
    const Vec2<int> c = {42, 42};

    SECTION("Vector absolute value")
    {
        REQUIRE(a.abs() == Vec2<int>{42, 42});
    }

    SECTION("Vector min value")
    {
        REQUIRE(Vec2<int>::min(a, b) == Vec2<int>{-42, -42});
    }

    SECTION("Vector max value")
    {
        REQUIRE(Vec2<int>::max(a, b) == Vec2<int>{42, 42});
    }

    SECTION("Vectors element wise less than comparison")
    {
        REQUIRE(a.lt(b) == Vec2<bool>{false, true});
        REQUIRE(b.lt(a) == Vec2<bool>{true, false});
    }

    SECTION("Vectors element wise greater than comparison")
    {
        REQUIRE(a.gt(b) == Vec2<bool>{true, false});
        REQUIRE(b.gt(a) == Vec2<bool>{false, true});
    }

    SECTION("Vectors element wise less or equal comparison")
    {
        REQUIRE(a.le(b) == a.lt(b));
        REQUIRE(b.le(a) == b.lt(a));
        REQUIRE(a.le(c) == Vec2<bool>(true, true));
        REQUIRE(b.le(c) == Vec2<bool>(true, true));
    }

    SECTION("Vectors element wise greater or equal comparison")
    {
        REQUIRE(a.ge(b) == a.gt(b));
        REQUIRE(b.ge(a) == b.gt(a));
        REQUIRE(a.ge(c) == Vec2<bool>(true, false));
        REQUIRE(b.ge(c) == Vec2<bool>(false, true));
    }

    SECTION("Vector element wise clamp")
    {
        REQUIRE(a.clamp({0, 0}, {42, 42}) == Vec2<int>{42, 0});
        REQUIRE(b.clamp({-50, -50}, {0, 0}) == Vec2<int>{-42, 0});
        REQUIRE(b.clamp({-50, -50}, {50, 50}) == b);
    }

    SECTION("Vector element type cast")
    {
        Vec2<float> b = {static_cast<float>(a.x), static_cast<float>(a.y)};
        auto a2 = a.into<float>();
        REQUIRE(typeid(a2.x) == typeid(float));
        REQUIRE(typeid(a2.y) == typeid(float));
        REQUIRE(a2 == b);
    }
}