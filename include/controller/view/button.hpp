#pragma once

#include "controller/view/color.hpp"
#include "controller/view/text.hpp"
#include <string>

namespace controller {

struct Button {
    Text text;
    Color backgroundColor = {100, 100, 100};
    float centerOffsetX = 0.0f;
    float centerOffsetY = 0.0f;
    float width = 100.0f;
    float height = 50.0f;
    bool isSelected = false;
    Color selectedColor = {0, 255, 0};
};

} // namespace controller