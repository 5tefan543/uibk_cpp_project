#pragma once

#include <string>

namespace game {

struct Map {
    float x = 0.0f;
    float y = 0.0f;
    float width = 1920.0f * 2.0f;
    float height = 1080.0f * 2.0f;
    std::string texturePath = "assets/maps/map.bmp";
    bool isSelected = false;
};

} // namespace game
