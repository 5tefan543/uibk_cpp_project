#pragma once
#include "color.hpp"
#include "geometry/rectangle.hpp"
#include "view/grid.hpp"
#include <optional>

namespace view {

struct Rectangle {
    geometry::Rectangle<float> rect =
        geometry::Rectangle<float>::centered(grid.getCenter(), grid.size / geometry::Vec2<float>{8, 20});
    Color borderColor = {250, 0, 10};
    float thickness = 20.0f;
    std::optional<Color> fillColor = std::nullopt;
};

} // namespace view