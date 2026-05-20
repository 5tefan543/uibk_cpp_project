#pragma once

#include <string>

namespace game {

enum class Direction { Left, Right, Up, Down };

struct Animation {
    Direction direction = Direction::Right;
    int currentFrame = 0;
    float frameTimer = 0.0f;
    float frameDuration = 0.2f; // seconds per animation frame
    int totalFrames = 4;        // Total animation frames in each direction
    std::string baseTexturePath = "";
};

} // namespace game
