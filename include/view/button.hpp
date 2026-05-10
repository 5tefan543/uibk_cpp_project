#pragma once

#include "color.hpp"
#include "grid.hpp"
#include "text.hpp"

namespace view {

struct Button {
    Text text;
    Color backgroundColor = {100, 100, 100};
    float width = gridWidth / 8;
    float height = gridHeight / 20;
    float gridX = (gridWidth / 2) - (width / 2);
    float gridY = (gridHeight / 2) - (height / 2);
    bool isSelected = false;
    Color selectedColor = {0, 255, 0};
};

} // namespace view