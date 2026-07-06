#include "game/ecs/systems/animation_system.hpp"
#include "config/animation_config_helper.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/enemy_attack_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/player_attack_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/velocity.hpp"
#include "geometry/vector.hpp"
#include "view/sprite.hpp"

#include <algorithm>
#include <cmath>

namespace game {

namespace {

constexpr float bossSpriteScale = 2.0f;

void applySpriteConfig(Registry &registry, Entity entity, const config::SpriteConfig &spriteConfig, float scale)
{
    view::Sprite &sprite = registry.getComponent<view::Sprite>(entity);
    sprite.imagePath = spriteConfig.texture.path;
    sprite.rect.size = spriteConfig.texture.size * scale;

    if (registry.hasComponent<HitBox>(entity)) {
        HitBox &hitBox = registry.getComponent<HitBox>(entity);
        hitBox.offset = spriteConfig.hitBox.offset * scale;
        hitBox.size = spriteConfig.hitBox.size * scale;
    }
}

float getSpriteScaleForEntity(Registry &registry, Entity entity)
{
    if (!registry.hasComponent<EnemyStats>(entity)) {
        return 1.0f;
    }

    const EnemyStats &enemyStats = registry.getComponent<EnemyStats>(entity);
    return enemyStats.enemyType == EnemyType::Boss ? bossSpriteScale : 1.0f;
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

    if (registry.hasComponent<Damage>(entity)) {
        const Damage &damage = registry.getComponent<Damage>(entity);
        if (damage.kind == DamageKind::Unicorn) {
            return config::AnimationConfigHelper::getUnicornAnimationFrame(
                config, config.playerClasses.ranged.attack.unicorn, animation.state, animation.direction,
                animation.currentFrame);
        }
    }

    if (registry.hasComponent<EnemyTag>(entity) && registry.hasComponent<EnemyStats>(entity)) {
        const EnemyStats &stats = registry.getComponent<EnemyStats>(entity);
        return config::AnimationConfigHelper::getEnemyAnimationFrame(config, stats.enemyType, animation.state,
                                                                     animation.direction, animation.currentFrame);
    }

    if (registry.hasComponent<Damage>(entity)) {
        const Damage &damage = registry.getComponent<Damage>(entity);

        const config::AreaAttackConfig *areaAttackConfig = nullptr;
        const config::ProjectileAttackConfig *projectileAttackConfig = nullptr;
        if (registry.hasComponent<PlayerAttackTag>(entity)) {
            CharacterType characterType = registry.getComponent<PlayerAttackTag>(entity).characterType;
            areaAttackConfig = &config.playerClasses.getByType(characterType).attack.area;
            projectileAttackConfig = &config.playerClasses.getByType(characterType).attack.projectile;
        }
        if (registry.hasComponent<EnemyAttackTag>(entity)) {
            EnemyType enemyType = registry.getComponent<EnemyAttackTag>(entity).enemyType;
            areaAttackConfig = &config.enemyClasses.getByType(enemyType).attack.area;
            projectileAttackConfig = &config.enemyClasses.getByType(enemyType).attack.projectile;
        }

        if (!areaAttackConfig && !projectileAttackConfig) {
            return std::nullopt;
        }

        switch (damage.kind) {
        case DamageKind::Projectile: {
            if (!projectileAttackConfig) {
                return std::nullopt;
            }
            return config::AnimationConfigHelper::getProjectileAnimationFrame(
                config, *projectileAttackConfig, animation.state, animation.direction, animation.currentFrame);
        }
        case DamageKind::Area: {
            if (!areaAttackConfig) {
                return std::nullopt;
            }
            return config::AnimationConfigHelper::getAreaAnimationFrame(config, *areaAttackConfig, animation.state,
                                                                        animation.direction, animation.currentFrame);
        }
        default:
            break;
        }
    }

    return std::nullopt;
}

} // namespace

void AnimationSystem::update(Registry &registry, const config::GameConfig &config, float dtSec)
{
    for (auto entity : registry.view<Animation, view::Sprite>()) {
        Animation &animation = registry.getComponent<Animation>(entity);

        const std::optional<config::AnimationFrame> frame =
            getAnimationFrameForEntity(registry, entity, config, animation);

        if (!frame.has_value()) {
            continue;
        }

        applySpriteConfig(registry, entity, frame->spriteConfig, getSpriteScaleForEntity(registry, entity));

        animation.frameTimer += dtSec;

        if (animation.frameTimer >= frame->frameDuration) {
            animation.frameTimer -= frame->frameDuration;
            animation.currentFrame = (animation.currentFrame + 1) % frame->totalFrames;
        }
    }
}

} // namespace game
