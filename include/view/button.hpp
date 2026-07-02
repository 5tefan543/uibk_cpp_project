#pragma once

#include "color.hpp"
#include "geometry/rectangle.hpp"
#include "geometry/vector.hpp"
#include "grid.hpp"
#include "text.hpp"

namespace view {

struct Button {
    size_t id = 0;
    Color backgroundColor = color::gray;
    geometry::Rectangle<float> rect =
        geometry::Rectangle<float>::centered(grid.getCenter(), grid.size / geometry::Vec2<float>{8, 20});
    Text text = {.position = rect.getCenter()};
    bool isSelected = false;
    Color selectedColor = color::green;
};

} // namespace view