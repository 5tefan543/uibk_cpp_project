#pragma once
#include <algorithm>
#include <cmath>
#include <optional>

template <typename T>
struct Rectangle {
    T x;
    T y;
    T width;
    T height;

    bool intersects(const Rectangle<T> &other) const
    {
        return !(x + width < other.x || x > other.x + other.width || y + height < other.y
                 || y > other.y + other.height);
    }

    bool contains(const Rectangle<T> &other) const
    {
        return x <= other.x && y <= other.y && x + width >= other.x + other.width
               && y + height >= other.y + other.height;
    }

    Rectangle<T> snapBack(const Rectangle<T> &boundary)
    {
        if (x < boundary.x) {
            x = boundary.x;
        }
        if (y < boundary.y) {
            y = boundary.y;
        }
        if (x + width > boundary.x + boundary.width) {
            x = boundary.x + boundary.width - width;
        }
        if (y + height > boundary.y + boundary.height) {
            y = boundary.y + boundary.height - height;
        }
        return *this;
    }

    std::optional<Rectangle<T>> findIntersection(const Rectangle<T> &other) const
    {
        if (!intersects(other)) {
            return std::nullopt;
        }

        T intersectX = std::max(x, other.x);
        T intersectY = std::max(y, other.y);
        T intersectWidth = std::min(x + width, other.x + other.width) - intersectX;
        T intersectHeight = std::min(y + height, other.y + other.height) - intersectY;

        return Rectangle<T>{intersectX, intersectY, intersectWidth, intersectHeight};
    }
};
