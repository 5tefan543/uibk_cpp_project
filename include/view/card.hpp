#pragma once

#include "color.hpp"
#include "view_item.hpp"
#include <vector>

namespace view {

struct Card {
    Color backgroundColor = {150, 150, 150};
    float width = gridWidth / 2;
    float height = gridHeight / 2;
    float gridX = (gridWidth / 2) - (width / 2);
    float gridY = (gridHeight / 2) - (height / 2);
    std::vector<ViewElement> elements;
};

} // namespace view