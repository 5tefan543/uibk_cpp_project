#include "game/ecs/systems/animation_system.hpp"
#include "config/animation_config_helper.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/velocity.hpp"
#include "geometry/vector.hpp"
#include "view/sprite.hpp"

#include <algorithm>
#include <cmath>

namespace game {

namespace {

void applySpriteConfig(Registry &registry, Entity entity, const config::SpriteConfig &spriteConfig)
{
    view::Sprite &sprite = registry.getComponent<view::Sprite>(entity);
    sprite.imagePath = spriteConfig.texture.path;
    sprite.rect.size = spriteConfig.texture.size;

    if (registry.hasComponent<HitBox>(entity)) {
        HitBox &hitBox = registry.getComponent<HitBox>(entity);
        hitBox.offset = spriteConfig.hitBox.offset;
        hitBox.size = spriteConfig.hitBox.size;
    }
}

std::optional<config::AnimationFrame> getAnimationFrameForEntity(Registry &registry, Entity entity,
                                                                 const config::GameConfig &config,
                                                                 const Animation &animation)
{
    if (registry.hasComponent<PlayerTag>(entity) && registry.hasComponent<PlayerStats>(entity)) {

        const PlayerStats &stats = registry.getComponent<PlayerStats>(entity);
        return config::AnimationConfigHelper::getPlayerAnimationFrame(config, stats.characterType, animation.state,
                                                                      animation.direction, animation.currentFrame);
    }

    if (registry.hasComponent<EnemyTag>(entity) && registry.hasComponent<EnemyStats>(entity)) {
        const EnemyStats &stats = registry.getComponent<EnemyStats>(entity);
        return config::AnimationConfigHelper::getEnemyAnimationFrame(config, stats.enemyType, animation.state,
                                                                     animation.direction, animation.currentFrame);
    }

    return std::nullopt;
}

} // namespace

void AnimationSystem::update(Registry &registry, const config::GameConfig &config, float dt)
{
    for (auto entity : registry.view<Animation, view::Sprite>()) {
        Animation &animation = registry.getComponent<Animation>(entity);

        const std::optional<config::AnimationFrame> frame =
            getAnimationFrameForEntity(registry, entity, config, animation);

        if (!frame.has_value()) {
            continue;
        }

        applySpriteConfig(registry, entity, frame->spriteConfig);

        animation.frameTimer += dt;

        if (animation.frameTimer >= frame->frameDuration) {
            animation.frameTimer -= frame->frameDuration;
            animation.currentFrame = (animation.currentFrame + 1) % frame->totalFrames;
        }
    }
}

} // namespace game
