#pragma once

#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>
#include <ostream>

namespace geometry {
template <typename T>
struct Vec2 {
    T x;
    T y;

    operator sf::Vector2<T>() const { return sf::Vector2<T>{x, y}; }

    Vec2<T> operator+(Vec2<T> other) const { return {x + other.x, y + other.y}; }
    Vec2<T> operator+(T scalar) const { return {x + scalar, y + scalar}; }

    Vec2<T> operator-(Vec2<T> other) const { return {x - other.x, y - other.y}; }
    Vec2<T> operator-(T scalar) const { return {x - scalar, y - scalar}; }

    Vec2<T> operator*(Vec2<T> other) const { return {x * other.x, y * other.y}; }
    Vec2<T> operator*(T scalar) const { return {x * scalar, y * scalar}; }

    Vec2<T> operator/(Vec2<T> other) const { return {x / other.x, y / other.y}; }
    Vec2<T> operator/(T scalar) const { return {x / scalar, y / scalar}; }

    void operator+=(T scalar)
    {
        x += scalar;
        y += scalar;
    };
    void operator-=(T scalar)
    {
        x -= scalar;
        y -= scalar;
    };
    void operator*=(T scalar)
    {
        x *= scalar;
        y *= scalar;
    };
    void operator/=(T scalar)
    {
        x /= scalar;
        y /= scalar;
    };
    void operator+=(Vec2<T> other)
    {
        x += other.x;
        y += other.y;
    };
    void operator-=(Vec2<T> other)
    {
        x -= other.x;
        y -= other.y;
    };
    void operator*=(Vec2<T> other)
    {
        x *= other.x;
        y *= other.y;
    };
    void operator/=(Vec2<T> other)
    {
        x /= other.x;
        y /= other.y;
    };
    bool operator==(Vec2<T> other) const { return x == other.x && y == other.y; };

    // Return type is the return type of std::sqrt() with its argument type being vector's x,y type.
    auto length() const { return std::sqrt(x * x + y * y); };
    void normalize()
    {
        const auto l = length();
        if (l != 0) {
            x /= l;
            y /= l;
        }
    }
    void setLength(T len)
    {
        const auto l = length();
        if (l != 0) {
            len /= l;
            x *= len;
            y *= len;
        }
    }

    Vec2<T> abs() const { return {std::abs(x), std::abs(y)}; };
    static Vec2<T> min(Vec2<T> a, Vec2<T> b) { return {std::min(a.x, b.x), std::min(a.y, b.y)}; };
    static Vec2<T> max(Vec2<T> a, Vec2<T> b) { return {std::max(a.x, b.x), std::max(a.y, b.y)}; };

    Vec2<T> clamp(Vec2<T> low, Vec2<T> high) const
    {
        return Vec2<T>{std::clamp(x, low.x, high.x), std::clamp(y, low.y, high.y)};
    }
    template <typename Into>
    Vec2<Into> into() const
    {
        return {static_cast<Into>(x), static_cast<Into>(y)};
    }

    // True if x and y are true
    template <typename T_ = T, std::enable_if_t<std::is_same_v<T_, bool>> * = nullptr>
    bool all()
    {
        return x && y;
    }

    // True if x or y is true
    template <typename T_ = T, std::enable_if_t<std::is_same_v<T_, bool>> * = nullptr>
    bool some()
    {
        return x || y;
    }
};

// Element-wise comparison
template <class T>
Vec2<bool> operator<(const Vec2<T> &l, const Vec2<T> r)
{
    return {l.x < r.x, l.y < r.y};
};

// Element-wise comparison
template <class T>
Vec2<bool> operator<=(const Vec2<T> &l, const Vec2<T> r)
{
    return {l.x <= r.x, l.y <= r.y};
};

// Element-wise comparison
template <class T>
Vec2<bool> operator>(const Vec2<T> &l, const Vec2<T> r)
{
    return {l.x > r.x, l.y > r.y};
};

// Element-wise comparison
template <class T>
Vec2<bool> operator>=(const Vec2<T> &l, const Vec2<T> r)
{
    return {l.x >= r.x, l.y >= r.y};
};

template <typename T>
std::ostream &operator<<(std::ostream &s, Vec2<T> v)
{
    return s << "{x: " << v.x << ", y: " << v.y << "}";
}

} // namespace geometry