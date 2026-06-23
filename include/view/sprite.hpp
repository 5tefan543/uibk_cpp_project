#pragma once

#include "geometry/rectangle.hpp"
#include <string>

namespace view {

struct Sprite {
    geometry::Rectangle<float> rect = {.position = {0, 0}, .size = {32.0f * 4.0f, 32.0f * 4.0f}};
    std::string imagePath = "";
    bool isSelected = false;
};

} // namespace view
