#include "config/game_config.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/enemy_attack_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/player_attack_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/entity.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/animation_system.hpp"
#include "shared/test_fixture.hpp"
#include "view/sprite.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {

config::SpriteConfig makeSpriteConfig(const std::string &path, float textureWidth, float textureHeight,
                                      float hitBoxOffsetX = 0.0f, float hitBoxOffsetY = 0.0f,
                                      float hitBoxWidth = 128.0f, float hitBoxHeight = 128.0f)
{
    config::SpriteConfig spriteConfig{};
    spriteConfig.texture.path = path;
    spriteConfig.texture.size = {textureWidth, textureHeight};
    spriteConfig.hitBox.offset = {hitBoxOffsetX, hitBoxOffsetY};
    spriteConfig.hitBox.size = {hitBoxWidth, hitBoxHeight};
    return spriteConfig;
}

config::GameConfig makeAnimationSystemTestConfig()
{
    config::GameConfig config{};

    config.fallbackSprite = makeSpriteConfig("fallback.png", 10.0f, 10.0f);

    config.playerClasses.melee.characterType = game::CharacterType::Melee;
    config.playerClasses.melee.animations.stateToStateConfig[game::AnimationState::Idle] = {
        .frameDuration = 0.25f,
        .moveSpeedMultiplier = 1.0f,
        .directionToFrames =
            {
                {
                    game::AnimationDirection::Right,
                    {
                        makeSpriteConfig("player_idle_right_1.png", 32.0f, 48.0f, 2.0f, 4.0f, 20.0f, 40.0f),
                        makeSpriteConfig("player_idle_right_2.png", 33.0f, 49.0f, 3.0f, 5.0f, 21.0f, 41.0f),
                    },
                },
                {
                    game::AnimationDirection::Left,
                    {
                        makeSpriteConfig("player_idle_left_1.png", 32.0f, 48.0f),
                        makeSpriteConfig("player_idle_left_2.png", 33.0f, 49.0f),
                    },
                },
            },
    };

    config.playerClasses.melee.attack.area.animations.stateToStateConfig[game::AnimationState::Idle] = {
        .frameDuration = 0.35f,
        .moveSpeedMultiplier = 1.0f,
        .directionToFrames =
            {
                {
                    game::AnimationDirection::None,
                    {
                        makeSpriteConfig("player_area_idle_1.png", 50.0f, 54.0f, 9.0f, 10.0f, 28.0f, 29.0f),
                        makeSpriteConfig("player_area_idle_2.png", 51.0f, 55.0f, 11.0f, 12.0f, 30.0f, 31.0f),
                    },
                },
            },
    };

    config.enemyClasses.blob.enemyType = game::EnemyType::Blob;
    config.enemyClasses.blob.animations.stateToStateConfig[game::AnimationState::Walk] = {
        .frameDuration = 0.5f,
        .moveSpeedMultiplier = 1.0f,
        .directionToFrames =
            {
                {
                    game::AnimationDirection::Left,
                    {
                        makeSpriteConfig("enemy_walk_left_1.png", 64.0f, 64.0f, 5.0f, 6.0f, 30.0f, 31.0f),
                        makeSpriteConfig("enemy_walk_left_2.png", 65.0f, 65.0f, 7.0f, 8.0f, 32.0f, 33.0f),
                    },
                },
            },
    };

    config.enemyClasses.blob.attack.area.animations.stateToStateConfig[game::AnimationState::Idle] = {
        .frameDuration = 0.4f,
        .moveSpeedMultiplier = 1.0f,
        .directionToFrames =
            {
                {
                    game::AnimationDirection::None,
                    {
                        makeSpriteConfig("enemy_area_idle_1.png", 40.0f, 44.0f, 1.0f, 2.0f, 18.0f, 22.0f),
                        makeSpriteConfig("enemy_area_idle_2.png", 41.0f, 45.0f, 3.0f, 4.0f, 19.0f, 23.0f),
                    },
                },
            },
    };

    config.playerClasses.ranged.characterType = game::CharacterType::Ranged;
    config.playerClasses.ranged.attack.unicorn.animations.stateToStateConfig[game::AnimationState::Walk] = {
        .frameDuration = 0.3f,
        .moveSpeedMultiplier = 1.0f,
        .directionToFrames =
            {
                {
                    game::AnimationDirection::Right,
                    {
                        makeSpriteConfig("unicorn_walk_right_1.png", 32.0f, 32.0f, 4.0f, 5.0f, 24.0f, 25.0f),
                        makeSpriteConfig("unicorn_walk_right_2.png", 33.0f, 33.0f, 6.0f, 7.0f, 26.0f, 27.0f),
                    },
                },
            },
    };

    return config;
}

game::PlayerStats makePlayerStats(game::CharacterType characterType)
{
    game::PlayerStats stats{};
    stats.characterType = characterType;
    return stats;
}

game::EnemyStats makeEnemyStats(game::EnemyType enemyType)
{
    game::EnemyStats stats{};
    stats.enemyType = enemyType;
    return stats;
}

} // namespace

TEST_CASE_METHOD(TestFixture, "AnimationSystem applies player sprite and hitbox from current animation frame")
{
    game::Registry registry;
    game::AnimationSystem system;
    const config::GameConfig config = makeAnimationSystemTestConfig();

    const game::Entity entity = registry.createEntity();

    registry.addComponent<game::PlayerTag>(entity, {});
    registry.addComponent<game::PlayerStats>(entity, makePlayerStats(game::CharacterType::Melee));
    registry.addComponent<game::Animation>(entity, {
                                                       .state = game::AnimationState::Idle,
                                                       .direction = game::AnimationDirection::Right,
                                                       .currentFrame = 0,
                                                       .frameTimer = 0.0f,
                                                   });
    registry.addComponent<view::Sprite>(entity, {});
    registry.addComponent<game::HitBox>(entity, {});

    system.update(registry, config, 0.0f);

    const auto &sprite = registry.getComponent<view::Sprite>(entity);
    const auto &hitBox = registry.getComponent<game::HitBox>(entity);
    const auto &animation = registry.getComponent<game::Animation>(entity);

    REQUIRE(sprite.imagePath == "player_idle_right_1.png");
    REQUIRE(sprite.rect.size.x == Catch::Approx(32.0f));
    REQUIRE(sprite.rect.size.y == Catch::Approx(48.0f));

    REQUIRE(hitBox.offset.x == Catch::Approx(2.0f));
    REQUIRE(hitBox.offset.y == Catch::Approx(4.0f));
    REQUIRE(hitBox.size.x == Catch::Approx(20.0f));
    REQUIRE(hitBox.size.y == Catch::Approx(40.0f));

    REQUIRE(animation.currentFrame == 0);
    REQUIRE(animation.frameTimer == Catch::Approx(0.0f));
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem advances frame when configured frame duration is reached")
{
    game::Registry registry;
    game::AnimationSystem system;
    const config::GameConfig config = makeAnimationSystemTestConfig();

    const game::Entity entity = registry.createEntity();

    registry.addComponent<game::PlayerTag>(entity, {});
    registry.addComponent<game::PlayerStats>(entity, makePlayerStats(game::CharacterType::Melee));
    registry.addComponent<game::Animation>(entity, {
                                                       .state = game::AnimationState::Idle,
                                                       .direction = game::AnimationDirection::Right,
                                                       .currentFrame = 0,
                                                       .frameTimer = 0.0f,
                                                   });
    registry.addComponent<view::Sprite>(entity, {});

    system.update(registry, config, 0.25f);

    const auto &animation = registry.getComponent<game::Animation>(entity);

    REQUIRE(animation.currentFrame == 1);
    REQUIRE(animation.frameTimer == Catch::Approx(0.0f));
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem keeps current frame while timer is below configured frame duration")
{
    game::Registry registry;
    game::AnimationSystem system;
    const config::GameConfig config = makeAnimationSystemTestConfig();

    const game::Entity entity = registry.createEntity();

    registry.addComponent<game::PlayerTag>(entity, {});
    registry.addComponent<game::PlayerStats>(entity, makePlayerStats(game::CharacterType::Melee));
    registry.addComponent<game::Animation>(entity, {
                                                       .state = game::AnimationState::Idle,
                                                       .direction = game::AnimationDirection::Right,
                                                       .currentFrame = 1,
                                                       .frameTimer = 0.0f,
                                                   });
    registry.addComponent<view::Sprite>(entity, {});

    system.update(registry, config, 0.1f);

    const auto &animation = registry.getComponent<game::Animation>(entity);
    const auto &sprite = registry.getComponent<view::Sprite>(entity);

    REQUIRE(animation.currentFrame == 1);
    REQUIRE(animation.frameTimer == Catch::Approx(0.1f));
    REQUIRE(sprite.imagePath == "player_idle_right_2.png");
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem preserves leftover frame time after advancing")
{
    game::Registry registry;
    game::AnimationSystem system;
    const config::GameConfig config = makeAnimationSystemTestConfig();

    const game::Entity entity = registry.createEntity();

    registry.addComponent<game::PlayerTag>(entity, {});
    registry.addComponent<game::PlayerStats>(entity, makePlayerStats(game::CharacterType::Melee));
    registry.addComponent<game::Animation>(entity, {
                                                       .state = game::AnimationState::Idle,
                                                       .direction = game::AnimationDirection::Right,
                                                       .currentFrame = 0,
                                                       .frameTimer = 0.2f,
                                                   });
    registry.addComponent<view::Sprite>(entity, {});

    system.update(registry, config, 0.1f);

    const auto &animation = registry.getComponent<game::Animation>(entity);

    REQUIRE(animation.currentFrame == 1);
    REQUIRE(animation.frameTimer == Catch::Approx(0.05f));
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem wraps current frame at configured total frame count")
{
    game::Registry registry;
    game::AnimationSystem system;
    const config::GameConfig config = makeAnimationSystemTestConfig();

    const game::Entity entity = registry.createEntity();

    registry.addComponent<game::PlayerTag>(entity, {});
    registry.addComponent<game::PlayerStats>(entity, makePlayerStats(game::CharacterType::Melee));
    registry.addComponent<game::Animation>(entity, {
                                                       .state = game::AnimationState::Idle,
                                                       .direction = game::AnimationDirection::Right,
                                                       .currentFrame = 1,
                                                       .frameTimer = 0.0f,
                                                   });
    registry.addComponent<view::Sprite>(entity, {});

    system.update(registry, config, 0.25f);

    const auto &animation = registry.getComponent<game::Animation>(entity);

    REQUIRE(animation.currentFrame == 0);
    REQUIRE(animation.frameTimer == Catch::Approx(0.0f));
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem applies enemy animation config")
{
    game::Registry registry;
    game::AnimationSystem system;
    const config::GameConfig config = makeAnimationSystemTestConfig();

    const game::Entity entity = registry.createEntity();

    registry.addComponent<game::EnemyTag>(entity, {});
    registry.addComponent<game::EnemyStats>(entity, makeEnemyStats(game::EnemyType::Blob));
    registry.addComponent<game::Animation>(entity, {
                                                       .state = game::AnimationState::Walk,
                                                       .direction = game::AnimationDirection::Left,
                                                       .currentFrame = 0,
                                                       .frameTimer = 0.0f,
                                                   });
    registry.addComponent<view::Sprite>(entity, {});
    registry.addComponent<game::HitBox>(entity, {});

    system.update(registry, config, 0.0f);

    const auto &sprite = registry.getComponent<view::Sprite>(entity);
    const auto &hitBox = registry.getComponent<game::HitBox>(entity);

    REQUIRE(sprite.imagePath == "enemy_walk_left_1.png");
    REQUIRE(sprite.rect.size.x == Catch::Approx(64.0f));
    REQUIRE(sprite.rect.size.y == Catch::Approx(64.0f));

    REQUIRE(hitBox.offset.x == Catch::Approx(5.0f));
    REQUIRE(hitBox.offset.y == Catch::Approx(6.0f));
    REQUIRE(hitBox.size.x == Catch::Approx(30.0f));
    REQUIRE(hitBox.size.y == Catch::Approx(31.0f));
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem applies sprite config when entity has no hitbox")
{
    game::Registry registry;
    game::AnimationSystem system;
    const config::GameConfig config = makeAnimationSystemTestConfig();

    const game::Entity entity = registry.createEntity();

    registry.addComponent<game::PlayerTag>(entity, {});
    registry.addComponent<game::PlayerStats>(entity, makePlayerStats(game::CharacterType::Melee));
    registry.addComponent<game::Animation>(entity, {
                                                       .state = game::AnimationState::Idle,
                                                       .direction = game::AnimationDirection::Right,
                                                       .currentFrame = 0,
                                                       .frameTimer = 0.0f,
                                                   });
    registry.addComponent<view::Sprite>(entity, {});

    REQUIRE_NOTHROW(system.update(registry, config, 0.0f));

    const auto &sprite = registry.getComponent<view::Sprite>(entity);

    REQUIRE(sprite.imagePath == "player_idle_right_1.png");
    REQUIRE(sprite.rect.size.x == Catch::Approx(32.0f));
    REQUIRE(sprite.rect.size.y == Catch::Approx(48.0f));
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem skips entity when no animation frame can be resolved")
{
    game::Registry registry;
    game::AnimationSystem system;
    const config::GameConfig config = makeAnimationSystemTestConfig();

    const game::Entity entity = registry.createEntity();

    registry.addComponent<game::Animation>(entity, {
                                                       .state = game::AnimationState::Idle,
                                                       .direction = game::AnimationDirection::Right,
                                                       .currentFrame = 0,
                                                       .frameTimer = 0.0f,
                                                   });

    registry.addComponent<view::Sprite>(entity, {});
    auto &spriteBeforeUpdate = registry.getComponent<view::Sprite>(entity);
    spriteBeforeUpdate.imagePath = "unchanged.png";
    spriteBeforeUpdate.rect.size = {10.0f, 20.0f};

    system.update(registry, config, 1.0f);

    const auto &animation = registry.getComponent<game::Animation>(entity);
    const auto &sprite = registry.getComponent<view::Sprite>(entity);

    REQUIRE(animation.currentFrame == 0);
    REQUIRE(animation.frameTimer == Catch::Approx(0.0f));

    REQUIRE(sprite.imagePath == "unchanged.png");
    REQUIRE(sprite.rect.size.x == Catch::Approx(10.0f));
    REQUIRE(sprite.rect.size.y == Catch::Approx(20.0f));
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem applies area attack animation for player attack entities")
{
    game::Registry registry;
    game::AnimationSystem system;
    const config::GameConfig config = makeAnimationSystemTestConfig();

    const game::Entity attackEntity = registry.createEntity();

    registry.addComponent<game::PlayerAttackTag>(attackEntity, {.characterType = game::CharacterType::Melee});
    registry.addComponent<game::Damage>(attackEntity, {
                                                          .amount = 5.0f,
                                                          .pushBackForce = 0.0f,
                                                          .stunChance = 0.0f,
                                                          .kind = game::DamageKind::Area,
                                                          .params =
                                                              game::AreaDamage{
                                                                  .radius = 50.0f,
                                                                  .activeTimeSec = 1.0f,
                                                                  .elapsedSec = 0.0f,
                                                                  .initialHit = 0.5f,
                                                                  .damageTicks = 3,
                                                                  .elapsedSecSinceLastTick = 0.0f,
                                                              },
                                                      });
    registry.addComponent<game::Animation>(attackEntity, {
                                                             .state = game::AnimationState::Idle,
                                                             .direction = game::AnimationDirection::None,
                                                             .currentFrame = 0,
                                                             .frameTimer = 0.0f,
                                                         });
    registry.addComponent<view::Sprite>(attackEntity, {});
    registry.addComponent<game::HitBox>(attackEntity, {});

    system.update(registry, config, 0.0f);

    const auto &sprite = registry.getComponent<view::Sprite>(attackEntity);
    const auto &hitBox = registry.getComponent<game::HitBox>(attackEntity);
    const auto &animation = registry.getComponent<game::Animation>(attackEntity);

    REQUIRE(sprite.imagePath == "player_area_idle_1.png");
    REQUIRE(sprite.rect.size.x == Catch::Approx(50.0f));
    REQUIRE(sprite.rect.size.y == Catch::Approx(54.0f));

    REQUIRE(hitBox.offset.x == Catch::Approx(9.0f));
    REQUIRE(hitBox.offset.y == Catch::Approx(10.0f));
    REQUIRE(hitBox.size.x == Catch::Approx(28.0f));
    REQUIRE(hitBox.size.y == Catch::Approx(29.0f));

    REQUIRE(animation.currentFrame == 0);
    REQUIRE(animation.frameTimer == Catch::Approx(0.0f));
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem applies area attack animation for enemy attack entities")
{
    game::Registry registry;
    game::AnimationSystem system;
    const config::GameConfig config = makeAnimationSystemTestConfig();

    const game::Entity attackEntity = registry.createEntity();

    registry.addComponent<game::EnemyAttackTag>(attackEntity, {.enemyType = game::EnemyType::Blob});
    registry.addComponent<game::Damage>(attackEntity, {
                                                          .amount = 5.0f,
                                                          .pushBackForce = 0.0f,
                                                          .stunChance = 0.0f,
                                                          .kind = game::DamageKind::Area,
                                                          .params =
                                                              game::AreaDamage{
                                                                  .radius = 50.0f,
                                                                  .activeTimeSec = 1.0f,
                                                                  .elapsedSec = 0.0f,
                                                                  .initialHit = 0.5f,
                                                                  .damageTicks = 3,
                                                                  .elapsedSecSinceLastTick = 0.0f,
                                                              },
                                                      });
    registry.addComponent<game::Animation>(attackEntity, {
                                                             .state = game::AnimationState::Idle,
                                                             .direction = game::AnimationDirection::None,
                                                             .currentFrame = 0,
                                                             .frameTimer = 0.0f,
                                                         });
    registry.addComponent<view::Sprite>(attackEntity, {});
    registry.addComponent<game::HitBox>(attackEntity, {});

    system.update(registry, config, 0.0f);

    const auto &sprite = registry.getComponent<view::Sprite>(attackEntity);
    const auto &hitBox = registry.getComponent<game::HitBox>(attackEntity);
    const auto &animation = registry.getComponent<game::Animation>(attackEntity);

    REQUIRE(sprite.imagePath == "enemy_area_idle_1.png");
    REQUIRE(sprite.rect.size.x == Catch::Approx(40.0f));
    REQUIRE(sprite.rect.size.y == Catch::Approx(44.0f));

    REQUIRE(hitBox.offset.x == Catch::Approx(1.0f));
    REQUIRE(hitBox.offset.y == Catch::Approx(2.0f));
    REQUIRE(hitBox.size.x == Catch::Approx(18.0f));
    REQUIRE(hitBox.size.y == Catch::Approx(22.0f));

    REQUIRE(animation.currentFrame == 0);
    REQUIRE(animation.frameTimer == Catch::Approx(0.0f));
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem advances area attack animation frame")
{
    game::Registry registry;
    game::AnimationSystem system;
    const config::GameConfig config = makeAnimationSystemTestConfig();

    const game::Entity attackEntity = registry.createEntity();

    registry.addComponent<game::PlayerAttackTag>(attackEntity, {.characterType = game::CharacterType::Melee});
    registry.addComponent<game::Damage>(attackEntity, {
                                                          .amount = 5.0f,
                                                          .pushBackForce = 0.0f,
                                                          .stunChance = 0.0f,
                                                          .kind = game::DamageKind::Area,
                                                          .params = game::AreaDamage{},
                                                      });
    registry.addComponent<game::Animation>(attackEntity, {
                                                             .state = game::AnimationState::Idle,
                                                             .direction = game::AnimationDirection::None,
                                                             .currentFrame = 0,
                                                             .frameTimer = 0.0f,
                                                         });
    registry.addComponent<view::Sprite>(attackEntity, {});

    system.update(registry, config, 0.35f);

    const auto &animation = registry.getComponent<game::Animation>(attackEntity);

    REQUIRE(animation.currentFrame == 1);
    REQUIRE(animation.frameTimer == Catch::Approx(0.0f));
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem skips area damage entity when attack tag is missing")
{
    game::Registry registry;
    game::AnimationSystem system;
    const config::GameConfig config = makeAnimationSystemTestConfig();

    const game::Entity attackEntity = registry.createEntity();

    registry.addComponent<game::Damage>(attackEntity, {
                                                          .amount = 3.0f,
                                                          .pushBackForce = 0.0f,
                                                          .stunChance = 0.0f,
                                                          .kind = game::DamageKind::Area,
                                                          .params = game::AreaDamage{},
                                                      });
    registry.addComponent<game::Animation>(attackEntity, {
                                                             .state = game::AnimationState::Idle,
                                                             .direction = game::AnimationDirection::None,
                                                             .currentFrame = 0,
                                                             .frameTimer = 0.1f,
                                                         });
    registry.addComponent<view::Sprite>(attackEntity, {});

    auto &spriteBeforeUpdate = registry.getComponent<view::Sprite>(attackEntity);
    spriteBeforeUpdate.imagePath = "unchanged_attack.png";
    spriteBeforeUpdate.rect.size = {11.0f, 22.0f};

    system.update(registry, config, 0.2f);

    const auto &animation = registry.getComponent<game::Animation>(attackEntity);
    const auto &sprite = registry.getComponent<view::Sprite>(attackEntity);

    REQUIRE(animation.currentFrame == 0);
    REQUIRE(animation.frameTimer == Catch::Approx(0.1f));

    REQUIRE(sprite.imagePath == "unchanged_attack.png");
    REQUIRE(sprite.rect.size.x == Catch::Approx(11.0f));
    REQUIRE(sprite.rect.size.y == Catch::Approx(22.0f));
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem applies unicorn damage animation config")
{
    game::Registry registry;
    game::AnimationSystem system;
    const config::GameConfig config = makeAnimationSystemTestConfig();

    const game::Entity unicorn = registry.createEntity();

    registry.addComponent<game::Damage>(unicorn, {
                                                     .amount = 15.0f,
                                                     .pushBackForce = 0.0f,
                                                     .stunChance = 0.0f,
                                                     .kind = game::DamageKind::Unicorn,
                                                     .params = game::UnicornDamage{},
                                                 });
    registry.addComponent<game::Animation>(unicorn, {
                                                        .state = game::AnimationState::Walk,
                                                        .direction = game::AnimationDirection::Right,
                                                        .currentFrame = 0,
                                                        .frameTimer = 0.0f,
                                                    });
    registry.addComponent<view::Sprite>(unicorn, {});
    registry.addComponent<game::HitBox>(unicorn, {});

    system.update(registry, config, 0.0f);

    const auto &sprite = registry.getComponent<view::Sprite>(unicorn);
    const auto &hitBox = registry.getComponent<game::HitBox>(unicorn);
    const auto &animation = registry.getComponent<game::Animation>(unicorn);

    REQUIRE(sprite.imagePath == "unicorn_walk_right_1.png");
    REQUIRE(sprite.rect.size.x == Catch::Approx(32.0f));
    REQUIRE(sprite.rect.size.y == Catch::Approx(32.0f));

    REQUIRE(hitBox.offset.x == Catch::Approx(4.0f));
    REQUIRE(hitBox.offset.y == Catch::Approx(5.0f));
    REQUIRE(hitBox.size.x == Catch::Approx(24.0f));
    REQUIRE(hitBox.size.y == Catch::Approx(25.0f));

    REQUIRE(animation.currentFrame == 0);
    REQUIRE(animation.frameTimer == Catch::Approx(0.0f));
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem advances unicorn damage animation frame")
{
    game::Registry registry;
    game::AnimationSystem system;
    const config::GameConfig config = makeAnimationSystemTestConfig();

    const game::Entity unicorn = registry.createEntity();

    registry.addComponent<game::Damage>(unicorn, {
                                                     .amount = 15.0f,
                                                     .pushBackForce = 0.0f,
                                                     .stunChance = 0.0f,
                                                     .kind = game::DamageKind::Unicorn,
                                                     .params = game::UnicornDamage{},
                                                 });
    registry.addComponent<game::Animation>(unicorn, {
                                                        .state = game::AnimationState::Walk,
                                                        .direction = game::AnimationDirection::Right,
                                                        .currentFrame = 0,
                                                        .frameTimer = 0.0f,
                                                    });
    registry.addComponent<view::Sprite>(unicorn, {});

    system.update(registry, config, 0.3f);

    const auto &animation = registry.getComponent<game::Animation>(unicorn);

    REQUIRE(animation.currentFrame == 1);
    REQUIRE(animation.frameTimer == Catch::Approx(0.0f));
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem skips non-unicorn damage entity")
{
    game::Registry registry;
    game::AnimationSystem system;
    const config::GameConfig config = makeAnimationSystemTestConfig();

    const game::Entity damageEntity = registry.createEntity();

    registry.addComponent<game::Damage>(damageEntity, {
                                                          .amount = 5.0f,
                                                          .pushBackForce = 0.0f,
                                                          .stunChance = 0.0f,
                                                          .kind = game::DamageKind::Projectile,
                                                          .params = game::ProjectileDamage{},
                                                      });
    registry.addComponent<game::Animation>(damageEntity, {
                                                             .state = game::AnimationState::Walk,
                                                             .direction = game::AnimationDirection::Right,
                                                             .currentFrame = 0,
                                                             .frameTimer = 0.1f,
                                                         });
    registry.addComponent<view::Sprite>(damageEntity, {});

    auto &spriteBeforeUpdate = registry.getComponent<view::Sprite>(damageEntity);
    spriteBeforeUpdate.imagePath = "unchanged_damage.png";
    spriteBeforeUpdate.rect.size = {11.0f, 22.0f};

    system.update(registry, config, 0.3f);

    const auto &animation = registry.getComponent<game::Animation>(damageEntity);
    const auto &sprite = registry.getComponent<view::Sprite>(damageEntity);

    REQUIRE(animation.currentFrame == 0);
    REQUIRE(animation.frameTimer == Catch::Approx(0.1f));

    REQUIRE(sprite.imagePath == "unchanged_damage.png");
    REQUIRE(sprite.rect.size.x == Catch::Approx(11.0f));
    REQUIRE(sprite.rect.size.y == Catch::Approx(22.0f));
}