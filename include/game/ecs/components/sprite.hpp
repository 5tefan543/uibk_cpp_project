#pragma once

#include <string>

namespace game {

enum class Direction { Left, Right, Up, Down };

// Sprite component that tracks animation state and direction
struct Sprite {
    Direction direction = Direction::Right;
    int currentFrame = 0;
    float frameTimer = 0.0f;
    float frameDuration = 0.2f;                         // seconds per animation frame
    int totalFrames = 4;                                // Total animation frames in each direction
    std::string baseTexturePath = "assets/characters/"; // Base path for textures
    float width = 32.0f * 4.0f;
    float height = 32.0f * 4.0f;
    bool isSelected = false;
};

} // namespace game
