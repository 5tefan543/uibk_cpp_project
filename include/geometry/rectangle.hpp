#pragma once
#include <algorithm>
#include <cmath>
#include <optional>

#include "geometry/vector.hpp"
namespace geometry {

template <typename T>
struct Rectangle {
    Vec2<T> position;
    Vec2<T> size;

    Rectangle<T> static centered(Vec2<T> center, Vec2<T> size) { return {center - (size / 2), size}; }
    Vec2<T> getCenter() const { return {position + (size / 2)}; }
    void centerizeY(float yCenter) { position.y = yCenter - size.y / 2; }
    void centerizeX(float xCenter) { position.x = xCenter - size.x / 2; }

    bool intersects(const Rectangle<T> &other) const
    {
        return !(position.x + size.x < other.position.x          // other on the right?
                 || position.x > other.position.x + other.size.x // other on the left?
                 || position.y + size.y < other.position.y       // other below?
                 || position.y > other.position.y + other.size.y // other above?
        );
    }

    bool contains(const Rectangle<T> &other) const
    {
        return (position <= other.position).all() && (position + size >= other.position + other.size).all();
    }

    // Assuming that sizes are not negative
    bool contains(const Vec2<T> &point) const { return (point >= position).all() && (point <= position + size).all(); }

    Rectangle<T> snapBack(const Rectangle<T> &boundary)
    {
        if (position.x < boundary.position.x) {
            position.x = boundary.position.x;
        }
        if (position.y < boundary.position.y) {
            position.y = boundary.position.y;
        }

        if (position.x + size.x > boundary.position.x + boundary.size.x) {
            position.x = boundary.position.x + boundary.size.x - size.x;
        }
        if (position.y + size.y > boundary.position.y + boundary.size.y) {
            position.y = boundary.position.y + boundary.size.y - size.y;
        }
        return *this;
    }

    std::optional<Rectangle<T>> findIntersection(const Rectangle<T> &other) const
    {
        using geometry::Vec2;
        if (!intersects(other)) {
            return std::nullopt;
        }

        Vec2<T> intersect = Vec2<T>::max(position, other.position);
        Vec2<T> intersectWidth = Vec2<T>::min(position + size, other.position + other.size) - intersect;

        return Rectangle<T>{intersect, intersectWidth};
    }
};
} // namespace geometry
