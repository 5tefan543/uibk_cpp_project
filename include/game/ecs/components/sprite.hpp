#pragma once

#include <string>

namespace game {

enum class Direction { Left, Right, Up, Down};

// Sprite component that tracks animation state and direction
struct Sprite {
    Direction direction = Direction::Right;
    int currentFrame = 0;
    float frameTimer = 0.0f;
    float frameDuration = 0.2f;                         // seconds per frame
    int totalFrames = 4;                                // Total animation frames in each direction
    std::string baseTexturePath = "assets/characters/"; // Base path for textures
};

} // namespace game
