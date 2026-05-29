#include "game/ecs/systems/animation_system.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/velocity.hpp"
#include "view/sprite.hpp"

#include <algorithm>
#include <cmath>

namespace game {

namespace {

struct OverwriteRenderConfig {
    const std::string *texturePath;
    float frameDuration;
    int totalFrames;
};

OverwriteRenderConfig getOverwriteRenderConfig(const Animation &animation)
{
    switch (animation.overrideState) {
    case AnimationOverrideState::Attack:
        return {&animation.attackTexturePath, animation.attackFrameDuration, animation.attackTotalFrames};
    case AnimationOverrideState::Death:
        return {&animation.deathTexturePath, animation.deathFrameDuration, animation.deathTotalFrames};
    default:
        return {nullptr, 0.0f, 0};
    }
}

} // namespace

void AnimationSystem::update(Registry &registry, float dt)
{
    for (auto entity : registry.view<Animation, view::Sprite>()) {
        Animation &animation = registry.getComponent<Animation>(entity);
        view::Sprite &sprite = registry.getComponent<view::Sprite>(entity);

        const OverwriteRenderConfig overwriteConfig = getOverwriteRenderConfig(animation);
        if (overwriteConfig.texturePath != nullptr && !overwriteConfig.texturePath->empty()) {
            animation.overrideTimeRemaining = std::max(0.0f, animation.overrideTimeRemaining - dt);
            animation.direction = animation.overrideDirection;

            animation.frameTimer += dt;
            if (animation.frameTimer >= overwriteConfig.frameDuration) {
                animation.frameTimer -= overwriteConfig.frameDuration;
                animation.currentFrame = (animation.currentFrame + 1) % std::max(1, overwriteConfig.totalFrames);
            }

            std::string directionStr = (animation.overrideDirection == Direction::Left) ? "left" : "right";
            int frameNum = animation.currentFrame + 1;
            sprite.imagePath = *overwriteConfig.texturePath + directionStr + "_" + std::to_string(frameNum) + ".png";

            if (animation.overrideTimeRemaining <= 0.0f) {
                animation.overrideState = AnimationOverrideState::None;
                animation.frameTimer = 0.0f;
                animation.currentFrame = 0;
            }

            continue;
        }

        if (registry.hasComponent<Velocity>(entity)) {
            const Velocity &velocity = registry.getComponent<Velocity>(entity);

            bool isMoving = std::abs(velocity.dx) > 0.1f || std::abs(velocity.dy) > 0.1f;

            if (std::abs(velocity.dx) > 0.1f) {
                animation.direction = velocity.dx > 0 ? Direction::Right : Direction::Left;
            }

            // Only update animation frame if moving
            if (isMoving) {
                animation.frameTimer += dt;
                if (animation.frameTimer >= animation.frameDuration) {
                    animation.frameTimer -= animation.frameDuration;
                    animation.currentFrame = (animation.currentFrame + 1) % animation.totalFrames;
                }
            } else {
                // Add Idle Anmimation
                animation.frameTimer = 0.0f;
            }
        }

        // Update sprite image path based on current animation state
        std::string directionStr = (animation.direction == Direction::Left) ? "left" : "right";
        int frameNum = animation.currentFrame + 1; // Frames are 1-indexed in filenames
        sprite.imagePath = animation.baseTexturePath + directionStr + "_" + std::to_string(frameNum) + ".png";
    }
}

} // namespace game
