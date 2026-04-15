#pragma once

#include <string>

namespace controller {

struct Sprite {
    float x = 0.0f;
    float y = 0.0f;
    std::string imagePath; // Path to the image file for current frame
    float width = 32.0f;
    float height = 32.0f;
    float scale = 1.0f; // Scale factor (1.0 = no scale, 4.0 = 4x, 2.0 = 2x)
    bool isMap = false; // True if this is a map (no camera offset applied)
};

} // namespace controller
