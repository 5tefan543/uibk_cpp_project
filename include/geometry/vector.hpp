#include <cmath>
#include <ostream>

template <typename T>
struct Vec2 {
    T x;
    T y;

    Vec2<T> operator+(Vec2<T> other) const { return Vec2{x + other.x, y + other.y}; }
    Vec2<T> operator+(T scalar) const { return Vec2{x + scalar, y + scalar}; }

    Vec2<T> operator-(Vec2<T> other) const { return Vec2{x - other.x, y - other.y}; }
    Vec2<T> operator-(T scalar) const { return Vec2{x - scalar, y - scalar}; }

    Vec2<T> operator*(Vec2<T> other) const { return Vec2{x * other.x, y * other.y}; }
    Vec2<T> operator*(T scalar) const { return Vec2{x * scalar, y * scalar}; }

    Vec2<T> operator/(Vec2<T> other) const { return Vec2{x / other.x, y / other.y}; }
    Vec2<T> operator/(T scalar) const { return Vec2{x / scalar, y / scalar}; }

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
    void setLenght(T len)
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

    // Return (this->x < b.y, this->y < b.y)
    Vec2<bool> lt(Vec2<T> b) const { return {x < b.x, y < b.y}; };

    // Return (this->x > b.y, this->y > b.y)
    Vec2<bool> gt(Vec2<T> b) const { return {x > b.x, y > b.y}; };

    // Return (this->x <= b.y, this->y <= b.y)
    Vec2<bool> le(Vec2<T> b) const { return {x <= b.x, y <= b.y}; };

    // Return (this->x >= b.y, this->y >= b.y)
    Vec2<bool> ge(Vec2<T> b) const { return {x >= b.x, y >= b.y}; };

    Vec2<T> clamp(Vec2<T> low, Vec2<T> high) const
    {
        return {std::clamp(x, low.x, high.x), std::clamp(y, low.y, high.y)};
    }
    template <typename Into>
    Vec2<Into> into() const
    {
        return {static_cast<Into>(x), static_cast<Into>(y)};
    }
};

template <typename T>
std::ostream &operator<<(std::ostream &s, Vec2<T> v)
{
    return s << "{x: " << v.x << ", y: " << v.y << "}";
}