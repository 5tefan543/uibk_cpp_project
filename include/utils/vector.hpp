#include <cmath>

template <typename T>
struct Vec2 {
    T x;
    T y;

    Vec2<T> operator+(Vec2<T> other) const { return Vec2{x + other.x, y + other.y}; }
    Vec2<T> operator-(Vec2<T> other) const { return Vec2{x - other.x, y - other.y}; }
    Vec2<T> operator*(T scalar) const { return Vec2{x * scalar, y * scalar}; }
    Vec2<T> operator/(Vec2<T> other) const { return Vec2{x / other.x, y / other.y}; }
    Vec2<T> operator/(T scalar) const { return Vec2{x / scalar, y / scalar}; }
    Vec2<T> operator*(Vec2<T> other) const { return Vec2{x * other.x, y * other.y}; }
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
    T length() const { return std::sqrt(x * x + y * y); };
    Vec2<T> abs() const { return {std::abs(x), std::abs(y)}; };
};