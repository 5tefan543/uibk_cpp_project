#pragma once

#include "controller/view/card.hpp"
#include "controller/view/color.hpp"
#include "controller/view/view_item.hpp"
#include <vector>

namespace controller {
struct View {
    Color backgroundColor = {20, 20, 20};
    std::vector<ViewItem> items;
    float cameraX = 0.0f; // Camera position for viewport culling
    float cameraY = 0.0f;
    float mapWidth = 1920.0f;
    float mapHeight = 1080.0f;
};
} // namespace controller