#pragma once
#include <algorithm>
#include <cmath>
#include <optional>

#include "geometry/vector.hpp"

template <typename T>
struct Rectangle {
    Vec2<T> position;
    Vec2<T> size;

    bool intersects(const Rectangle<T> &other) const
    {
        return !(position.x + size.x < other.position.x || position.x > other.position.x + other.size.x
                 || position.y + size.y < other.position.y || position.y > other.position.y + other.size.y);
    }

    bool contains(const Rectangle<T> &other) const
    {
        return position.x <= other.position.x && position.y <= other.position.y
               && position.x + size.x >= other.position.x + other.size.x
               && position.y + size.y >= other.position.y + other.size.y;
    }

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
