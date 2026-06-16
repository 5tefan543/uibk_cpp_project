#pragma once

#include <iostream>
#include <string>

namespace game {

enum class AnimationDirection { None, Left, Right, Up, Down };

enum class AnimationState { Idle, Walk, Attack, Hit, Death };

inline const char *toString(AnimationDirection direction)
{
    switch (direction) {
    case AnimationDirection::None:
        return "None";
    case AnimationDirection::Left:
        return "Left";
    case AnimationDirection::Right:
        return "Right";
    case AnimationDirection::Up:
        return "Up";
    case AnimationDirection::Down:
        return "Down";
    default:
        return "Unknown";
    }
}

inline const char *toString(game::AnimationState state)
{
    switch (state) {
    case game::AnimationState::Idle:
        return "Idle";
    case game::AnimationState::Walk:
        return "Walk";
    case game::AnimationState::Attack:
        return "Attack";
    case game::AnimationState::Hit:
        return "Hit";
    case game::AnimationState::Death:
        return "Death";
    default:
        return "Unknown";
    }
}

struct Animation {
    AnimationState state = AnimationState::Idle;
    AnimationDirection direction = AnimationDirection::None;
    size_t currentFrame = 0;
    float frameTimer = 0.0f;
    float stateTimeRemaining = 0.0f;
};

} // namespace game
