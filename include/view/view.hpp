#pragma once

#include "color.hpp"
#include "view_item.hpp"
#include <vector>

namespace view {
struct View {
    Color backgroundColor = {0, 0, 0};

    float cameraX = 0.0f;
    float cameraY = 0.0f;

    std::vector<ViewNode> nodes;
};
} // namespace view