#include "game/ecs/systems/animation_system.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/velocity.hpp"
#include "utils/vector.hpp"
#include "view/sprite.hpp"

#include <cmath>

namespace game {

void AnimationSystem::update(Registry &registry, float dt)
{
    for (auto entity : registry.view<Animation, view::Sprite, Velocity>()) {
        Animation &animation = registry.getComponent<Animation>(entity);
        const Velocity &velocity = registry.getComponent<Velocity>(entity);
        view::Sprite &sprite = registry.getComponent<view::Sprite>(entity);

        const bool isMoving = std::abs(velocity.x) > 0.1f || std::abs(velocity.y) > 0.1f;
        const bool isEnemy = registry.hasComponent<EnemyTag>(entity);
        const auto v = Vec2{velocity.x, velocity.y}.abs(); // TODO: Velocity use Vec2
        const bool isEnemyHorizMove = isEnemy && v.x >= v.y;
        if ((isEnemyHorizMove || !isEnemy) && std::abs(velocity.x) > 0.1f) {
            animation.direction = velocity.x > 0 ? Direction::Right : Direction::Left;
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

        // Update sprite image path based on current animation state
        const std::string directionStr = (animation.direction == Direction::Left) ? "left" : "right";
        const int frameNum = animation.currentFrame + 1; // Frames are 1-indexed in filenames
        sprite.imagePath = animation.baseTexturePath + directionStr + "_" + std::to_string(frameNum) + ".png";
    }
}

} // namespace game
