#pragma once

#include "color.hpp"
#include "font.hpp"
#include "grid.hpp"
#include <string>

namespace view {

struct Text {
    std::string text;
    Font font = Font::Default;
    unsigned int size = 30;
    Color color = {255, 255, 255};
    float gridX = gridWidth / 2;
    float gridY = gridHeight / 2;
    // Origin is topLeft of text bounding box
    float originOffsetX = 0.0f;
    float originOffsetY = 0.0f;
};

} // namespace view