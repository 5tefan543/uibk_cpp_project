#pragma once
#include "color.hpp"
#include "grid.hpp"

namespace view {

struct Rectangle {
    float width = gridWidth / 2;
    float height = gridHeight / 2;
    float gridX = (gridWidth / 2) - (width / 2);
    float gridY = (gridHeight / 2) - (height / 2);
    Color borderColor = {250, 0, 10};
    float thickness = 20.0f;
};

} // namespace view