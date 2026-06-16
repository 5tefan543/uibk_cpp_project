#pragma once

#include <string>

namespace view {

struct Sprite {
    float x = 0.0f;
    float y = 0.0f;
    std::string imagePath = "";
    float width;
    float height;
    bool isSelected = false;
};

} // namespace view
