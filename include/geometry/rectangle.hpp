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
        // TODO: use Vec operators
        return !(position.x + size.x < other.position.x || position.x > other.position.x + other.size.x
                 || position.y + size.y < other.position.y || position.y > other.position.y + other.size.y);
    }

    bool contains(const Rectangle<T> &other) const
    {
        // TODO: use Vec operators
        return position.x <= other.position.x && position.y <= other.position.y
               && position.x + size.x >= other.position.x + other.size.x
               && position.y + size.y >= other.position.y + other.size.y;
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
        // TODO: use Vec operators
        if (!intersects(other)) {
            return std::nullopt;
        }

        T intersectX = std::max(position.x, other.position.x);
        T intersectY = std::max(position.y, other.position.y);
        T intersectWidth = std::min(position.x + size.x, other.position.x + other.size.x) - intersectX;
        T intersectHeight = std::min(position.y + size.y, other.position.y + other.size.y) - intersectY;

        return Rectangle<T>{{intersectX, intersectY}, {intersectWidth, intersectHeight}};
    }
};
} // namespace geometry
