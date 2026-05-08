#pragma once

#include <string>

namespace game {

struct Map {
    float x = 0.0f;
    float y = 0.0f;
    float width = 1920.0f; // Map dimensions in game units
    float height = 1080.0f;
    float scale = 2.0f; // Map scaled by 2x
    std::string texturePath = "assets/maps/map.bmp";
    bool isSelected = false;
};

} // namespace game
