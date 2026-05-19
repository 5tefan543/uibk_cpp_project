#pragma once

#include <string>

namespace view {

struct Sprite {
    float x = 0.0f;
    float y = 0.0f;
    std::string imagePath;
    float width = 32.0f * 4.0f;
    float height = 32.0f * 4.0f;
    bool isSelected = false;
};

} // namespace view
