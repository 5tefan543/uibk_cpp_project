#pragma once

#include "controller/view/color.hpp"
#include "controller/view/font.hpp"
#include <string>

namespace controller {

struct Text {
    std::string text;
    Font font = Font::Default;
    unsigned int size = 30;
    Color color = {100, 100, 100};
    float centerOffsetX = 0.0f;
    float centerOffsetY = 0.0f;
    // Origin is topLeft of text bounding box
    float originOffsetX = 0.0f;
    float originOffsetY = 0.0f;
};

} // namespace controller