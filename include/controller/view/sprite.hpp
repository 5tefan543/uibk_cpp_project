#pragma once

#include <string>

namespace controller {

struct Sprite {
    float x = 0.0f;
    float y = 0.0f;
    std::string imagePath;
    float width = 32.0f;
    float height = 32.0f;
    float scale = 1.0f;
    bool isMap = false;
};

} // namespace controller
