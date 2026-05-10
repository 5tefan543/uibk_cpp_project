#pragma once

#include "card.hpp"
#include "color.hpp"
#include "view_item.hpp"
#include <vector>

namespace view {
struct View {
    Color backgroundColor = {0, 0, 0};
    std::vector<ViewItem> items;
    float cameraX = 0.0f; // Camera position for viewport culling
    float cameraY = 0.0f;
};
} // namespace view