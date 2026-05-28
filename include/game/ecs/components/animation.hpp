#pragma once

#include <string>

namespace game {

enum class Direction { Left, Right, Up, Down };

enum class AnimationOverrideState {
    None,
    Attack,
};

struct Animation {
    Direction direction = Direction::Right;
    int currentFrame = 0;
    float frameTimer = 0.0f;
    float frameDuration = 0.2f; // seconds per animation frame
    int totalFrames = 4;        // Total animation frames in each direction
    std::string baseTexturePath = "";

    AnimationOverrideState overrideState = AnimationOverrideState::None;
    float overrideTimeRemaining = 0.0f;
    Direction overrideDirection = Direction::Right;
    std::string attackTexturePath = "";
    float attackFrameDuration = 0.16f;
    int attackTotalFrames = 2;
    float attackMoveSpeedMultiplier = 0.5f;
};

} // namespace game
