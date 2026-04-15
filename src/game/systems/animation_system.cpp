#include "game/ecs/systems/animation_system.hpp"
#include "game/ecs/components/sprite.hpp"
#include "game/ecs/components/velocity.hpp"

#include <cmath>

namespace game {

void AnimationSystem::update(Registry &registry, float dt)
{
    for (auto entity : registry.view<Sprite, Velocity>()) {
        Sprite &sprite = registry.getComponent<Sprite>(entity);
        const Velocity &velocity = registry.getComponent<Velocity>(entity);

        bool isMoving = std::abs(velocity.dx) > 0.1f || std::abs(velocity.dy) > 0.1f;

        if (std::abs(velocity.dx) > 0.1f) {
            sprite.direction = velocity.dx > 0 ? Direction::Right : Direction::Left;
        }

        // Only update animation frame if moving
        if (isMoving) {
            sprite.frameTimer += dt;
            if (sprite.frameTimer >= sprite.frameDuration) {
                sprite.frameTimer -= sprite.frameDuration;
                sprite.currentFrame = (sprite.currentFrame + 1) % sprite.totalFrames;
            }
        } else {
            // Reset frame timer when idle
            sprite.frameTimer = 0.0f;
        }
    }
}

} // namespace game
