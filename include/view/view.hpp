#pragma once

#include "card.hpp"
#include "color.hpp"
#include "view_item.hpp"
#include <vector>

namespace view {
struct View {
    Color backgroundColor = color::black;
    geometry::Vec2<float> cameraPosition = {0.0f, 0.0f};
    std::vector<ViewNode> nodes;
};
} // namespace view