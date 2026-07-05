#include "config/game_config.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/player_attack_cooldown.hpp"
#include "game/ecs/components/player_attack_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sound.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/input_system.hpp"
#include "shared/test_fixture.hpp"
#include "view/sprite.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <variant>

namespace {

config::SpriteConfig makeSpriteConfig(const std::string &path, float width, float height)
{
    config::SpriteConfig spriteConfig{};
    spriteConfig.texture.path = path;
    spriteConfig.texture.size = {width, height};
    spriteConfig.hitBox.offset = {1.0f, 2.0f};
    spriteConfig.hitBox.size = {width - 2.0f, height - 2.0f};
    return spriteConfig;
}

void addAnimationState(config::AnimationConfig &animations, game::AnimationState state,
                       game::AnimationDirection direction, float frameDuration, float moveSpeedMultiplier,
                       std::vector<config::SpriteConfig> frames)
{
    auto &stateConfig = animations.stateToStateConfig[state];
    stateConfig.frameDuration = frameDuration;
    stateConfig.moveSpeedMultiplier = moveSpeedMultiplier;
    stateConfig.directionToFrames[direction] = std::move(frames);
}

void addDefaultPlayerAnimations(config::PlayerClassConfig &playerClass)
{
    addAnimationState(playerClass.animations, game::AnimationState::Idle, game::AnimationDirection::Right, 0.2f, 1.0f,
                      {
                          makeSpriteConfig("idle_right_1.png", 32.0f, 48.0f),
                          makeSpriteConfig("idle_right_2.png", 32.0f, 48.0f),
                      });

    addAnimationState(playerClass.animations, game::AnimationState::Idle, game::AnimationDirection::Left, 0.2f, 1.0f,
                      {
                          makeSpriteConfig("idle_left_1.png", 32.0f, 48.0f),
                          makeSpriteConfig("idle_left_2.png", 32.0f, 48.0f),
                      });

    addAnimationState(playerClass.animations, game::AnimationState::Walk, game::AnimationDirection::Right, 0.2f, 1.0f,
                      {
                          makeSpriteConfig("walk_right_1.png", 32.0f, 48.0f),
                          makeSpriteConfig("walk_right_2.png", 32.0f, 48.0f),
                      });

    addAnimationState(playerClass.animations, game::AnimationState::Walk, game::AnimationDirection::Left, 0.2f, 1.0f,
                      {
                          makeSpriteConfig("walk_left_1.png", 32.0f, 48.0f),
                          makeSpriteConfig("walk_left_2.png", 32.0f, 48.0f),
                      });

    addAnimationState(playerClass.animations, game::AnimationState::Attack, game::AnimationDirection::Right, 0.2f, 1.0f,
                      {
                          makeSpriteConfig("attack_right_1.png", 40.0f, 48.0f),
                          makeSpriteConfig("attack_right_2.png", 40.0f, 48.0f),
                      });

    addAnimationState(playerClass.animations, game::AnimationState::Attack, game::AnimationDirection::Left, 0.2f, 1.0f,
                      {
                          makeSpriteConfig("attack_left_1.png", 40.0f, 48.0f),
                          makeSpriteConfig("attack_left_2.png", 40.0f, 48.0f),
                      });
}

config::GameConfig makeInputSystemTestConfig()
{
    config::GameConfig config{};
    config.fallbackSprite = makeSpriteConfig("fallback.png", 16.0f, 16.0f);

    config.playerClasses.melee.characterType = game::CharacterType::Melee;
    config.playerClasses.melee.attack.amount = 12.0f;
    config.playerClasses.melee.attack.pushBackForce = 3.0f;
    config.playerClasses.melee.attack.stunChance = 0.25f;
    config.playerClasses.melee.attack.meleeArc.reach = 10.0f;
    config.playerClasses.melee.attack.meleeArc.hitBoxSize = {64.0f, 32.0f};
    config.playerClasses.melee.attack.meleeArc.activeTimePaddingSec = 0.1f;
    config.playerClasses.melee.attack.area.radius = 48.0f;
    config.playerClasses.melee.attack.area.activeTimeSec = 1.2f;
    config.playerClasses.melee.attack.area.initialHit = 0.5f;
    config.playerClasses.melee.attack.area.damageTicks = 3;
    config.playerClasses.melee.sounds.attack = "melee_attack.wav";
    config.playerClasses.melee.sounds.special = "melee_special.wav";
    addDefaultPlayerAnimations(config.playerClasses.melee);

    addAnimationState(config.playerClasses.melee.attack.area.animations, game::AnimationState::Idle,
                      game::AnimationDirection::None, 0.1f, 1.0f,
                      {
                          makeSpriteConfig("melee_area_idle_1.png", 32.0f, 32.0f),
                          makeSpriteConfig("melee_area_idle_2.png", 32.0f, 32.0f),
                      });

    config.playerClasses.ranged.characterType = game::CharacterType::Ranged;
    config.playerClasses.ranged.attack.amount = 8.0f;
    config.playerClasses.ranged.attack.pushBackForce = 2.0f;
    config.playerClasses.ranged.attack.stunChance = 0.1f;
    config.playerClasses.ranged.attack.projectile.velocityScale = 2.0f;
    config.playerClasses.ranged.sounds.attack = "ranged_attack.wav";
    config.playerClasses.ranged.sounds.special = "ranged_special.wav";
    addDefaultPlayerAnimations(config.playerClasses.ranged);

    addAnimationState(config.playerClasses.ranged.attack.projectile.animations, game::AnimationState::Idle,
                      game::AnimationDirection::None, 0.1f, 1.0f,
                      {
                          makeSpriteConfig("projectile_idle_1.png", 10.0f, 8.0f),
                      });

    config.playerClasses.ranged.attack.unicorn.velocityScale = 3.0f;
    addAnimationState(config.playerClasses.ranged.attack.unicorn.animations, game::AnimationState::Walk,
                      game::AnimationDirection::Right, 0.1f, 1.0f,
                      {
                          makeSpriteConfig("unicorn_walk_right_1.png", 32.0f, 32.0f),
                          makeSpriteConfig("unicorn_walk_right_2.png", 32.0f, 32.0f),
                      });

    return config;
}

game::PlayerStats makePlayerStats(game::CharacterType characterType)
{
    game::PlayerStats stats{};
    stats.characterType = characterType;
    stats.moveSpeed = 100.0f;
    stats.attackSpeed = 2.0f;        // cooldown = 0.5 seconds
    stats.specialAttackSpeed = 2.0f; // cooldown = 0.5 seconds
    stats.speedOfAttack = 50.0f;
    stats.attackRange = 3.0f;
    return stats;
}

view::Sprite makePlayerSprite()
{
    view::Sprite sprite{};
    sprite.imagePath = "player.png";
    sprite.rect.size = {32.0f, 48.0f};
    return sprite;
}

game::Animation makeAnimation(game::AnimationState state = game::AnimationState::Idle,
                              game::AnimationDirection direction = game::AnimationDirection::Right)
{
    game::Animation animation{};
    animation.state = state;
    animation.direction = direction;
    animation.currentFrame = 0;
    animation.frameTimer = 0.0f;
    animation.stateTimeRemaining = 0.0f;
    return animation;
}

game::Entity addPlayer(game::Registry &registry, game::CharacterType characterType = game::CharacterType::Melee)
{
    const game::Entity player = registry.createEntity();

    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<game::PlayerStats>(player, makePlayerStats(characterType));
    registry.addComponent<game::Velocity>(player, {});
    registry.addComponent<game::Position>(player, {100.0f, 100.0f});
    registry.addComponent<view::Sprite>(player, makePlayerSprite());
    registry.addComponent<game::Animation>(player, makeAnimation());
    registry.addComponent<game::PlayerAttackCooldown>(player, {});

    return player;
}

std::vector<game::Entity> getPlayerAttacks(game::Registry &registry)
{
    return registry.view<game::Damage, game::DamageTag, game::PlayerAttackTag>();
}

game::Entity requireSinglePlayerAttack(game::Registry &registry)
{
    const auto attacks = getPlayerAttacks(registry);

    REQUIRE(attacks.size() == 1);

    return attacks.front();
}

void requireNoPlayerAttacks(game::Registry &registry)
{
    REQUIRE(getPlayerAttacks(registry).empty());
}

void setAttackCooldowns(game::Registry &registry, game::Entity player, float attackRemainingSec,
                        float specialAttackRemainingSec)
{
    auto &cooldown = registry.getComponent<game::PlayerAttackCooldown>(player);
    cooldown.attackRemainingSec = attackRemainingSec;
    cooldown.specialAttackRemainingSec = specialAttackRemainingSec;
}

} // namespace

TEST_CASE_METHOD(TestFixture, "InputSystem normalizes diagonal player movement")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry);

    controller::InputState input{};
    input.rightHeld = true;
    input.downHeld = true;

    system.update(registry, config, input, 0.0f);

    const auto &velocity = registry.getComponent<game::Velocity>(player).v;

    const float expected = 100.0f / std::sqrt(2.0f);

    REQUIRE(velocity.x == Catch::Approx(expected));
    REQUIRE(velocity.y == Catch::Approx(expected));
}

TEST_CASE_METHOD(TestFixture, "InputSystem changes player animation to walk left on left movement")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry);

    auto &animationBefore = registry.getComponent<game::Animation>(player);
    animationBefore.state = game::AnimationState::Idle;
    animationBefore.direction = game::AnimationDirection::Right;
    animationBefore.currentFrame = 1;
    animationBefore.frameTimer = 0.15f;

    controller::InputState input{};
    input.leftHeld = true;

    system.update(registry, config, input, 0.0f);

    const auto &animation = registry.getComponent<game::Animation>(player);

    REQUIRE(animation.state == game::AnimationState::Walk);
    REQUIRE(animation.direction == game::AnimationDirection::Left);
    REQUIRE(animation.currentFrame == 0);
    REQUIRE(animation.frameTimer == Catch::Approx(0.0f));
}

TEST_CASE_METHOD(TestFixture, "InputSystem keeps current animation direction for vertical movement")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry);

    auto &animationBefore = registry.getComponent<game::Animation>(player);
    animationBefore.direction = game::AnimationDirection::Left;

    controller::InputState input{};
    input.upHeld = true;

    system.update(registry, config, input, 0.0f);

    const auto &animation = registry.getComponent<game::Animation>(player);

    REQUIRE(animation.state == game::AnimationState::Walk);
    REQUIRE(animation.direction == game::AnimationDirection::Left);
}

TEST_CASE_METHOD(TestFixture, "InputSystem applies animation move speed multiplier")
{
    game::Registry registry;
    game::InputSystem system;
    config::GameConfig config = makeInputSystemTestConfig();

    config.playerClasses.melee.animations.stateToStateConfig[game::AnimationState::Walk].moveSpeedMultiplier = 0.5f;

    const game::Entity player = addPlayer(registry);

    controller::InputState input{};
    input.rightHeld = true;

    system.update(registry, config, input, 0.0f);

    const auto &velocity = registry.getComponent<game::Velocity>(player).v;

    REQUIRE(velocity.x == Catch::Approx(50.0f));
    REQUIRE(velocity.y == Catch::Approx(0.0f));
}

TEST_CASE_METHOD(TestFixture, "InputSystem does not replace timed animation while time remains")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry);

    auto &animationBefore = registry.getComponent<game::Animation>(player);
    animationBefore.state = game::AnimationState::Attack;
    animationBefore.direction = game::AnimationDirection::Left;
    animationBefore.stateTimeRemaining = 0.5f;

    controller::InputState input{};
    input.rightHeld = true;

    system.update(registry, config, input, 0.1f);

    const auto &animation = registry.getComponent<game::Animation>(player);

    REQUIRE(animation.state == game::AnimationState::Attack);
    REQUIRE(animation.direction == game::AnimationDirection::Left);
    REQUIRE(animation.stateTimeRemaining == Catch::Approx(0.4f));
}

TEST_CASE_METHOD(TestFixture, "InputSystem spawns melee attack entity and starts attack animation")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry, game::CharacterType::Melee);

    controller::InputState input{};
    input.mouseLeftPressed = true;
    input.mouseGrid = {200.0f, 100.0f};

    system.update(registry, config, input, 1.0f);

    const auto attacks =
        registry.view<game::Damage, game::DamageTag, game::PlayerAttackTag, game::Position, game::HitBox>();

    REQUIRE(attacks.size() == 1);

    const game::Entity attackEntity = attacks.front();

    const auto &damage = registry.getComponent<game::Damage>(attackEntity);
    const auto &position = registry.getComponent<game::Position>(attackEntity).p;
    const auto &hitBox = registry.getComponent<game::HitBox>(attackEntity);

    REQUIRE(damage.amount == Catch::Approx(12.0f));
    REQUIRE(damage.pushBackForce == Catch::Approx(3.0f));
    REQUIRE(damage.stunChance == Catch::Approx(0.25f));
    REQUIRE(damage.kind == game::DamageKind::MeleeArc);

    const auto &meleeParams = std::get<game::MeleeArcDamage>(damage.params);

    REQUIRE(meleeParams.reach == Catch::Approx(10.0f));

    // Attack animation has 2 frames * 0.2 sec = 0.4 sec.
    // Melee active time adds 0.1 sec padding.
    REQUIRE(meleeParams.activeTimeSec == Catch::Approx(0.5f));
    REQUIRE(meleeParams.elapsedSec == Catch::Approx(0.0f));

    REQUIRE(position.x == Catch::Approx(100.0f));
    REQUIRE(position.y == Catch::Approx(100.0f));

    REQUIRE(hitBox.offset.x == Catch::Approx(16.0f));
    REQUIRE(hitBox.offset.y == Catch::Approx(-30.0f));
    REQUIRE(hitBox.size.x == Catch::Approx(124.0f));
    REQUIRE(hitBox.size.y == Catch::Approx(92.0f));

    const auto &playerAnimation = registry.getComponent<game::Animation>(player);

    REQUIRE(playerAnimation.state == game::AnimationState::Attack);
    REQUIRE(playerAnimation.direction == game::AnimationDirection::Right);
    REQUIRE(playerAnimation.currentFrame == 0);
    REQUIRE(playerAnimation.frameTimer == Catch::Approx(0.0f));
    REQUIRE(playerAnimation.stateTimeRemaining == Catch::Approx(0.4f));
}

TEST_CASE_METHOD(TestFixture, "InputSystem spawns projectile attack entity")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry, game::CharacterType::Ranged);

    controller::InputState input{};
    input.mouseLeftPressed = true;
    input.mouseGrid = {200.0f, 120.0f};

    system.update(registry, config, input, 1.0f);

    const auto projectiles = registry.view<game::Damage, game::DamageTag, game::PlayerAttackTag, game::Position,
                                           game::Velocity, game::HitBox, view::Sprite>();

    REQUIRE(projectiles.size() == 1);

    const game::Entity projectile = projectiles.front();

    const auto &damage = registry.getComponent<game::Damage>(projectile);
    const auto &position = registry.getComponent<game::Position>(projectile).p;
    const auto &velocity = registry.getComponent<game::Velocity>(projectile).v;
    const auto &sprite = registry.getComponent<view::Sprite>(projectile);
    const auto &hitBox = registry.getComponent<game::HitBox>(projectile);

    REQUIRE(damage.amount == Catch::Approx(8.0f));
    REQUIRE(damage.pushBackForce == Catch::Approx(2.0f));
    REQUIRE(damage.stunChance == Catch::Approx(0.1f));
    REQUIRE(damage.kind == game::DamageKind::Projectile);

    const auto &projectileParams = std::get<game::ProjectileDamage>(damage.params);

    REQUIRE(projectileParams.speed == Catch::Approx(50.0f));
    REQUIRE(projectileParams.maxRange == Catch::Approx(3.0f));
    REQUIRE(projectileParams.distanceTraveled == Catch::Approx(0.0f));
    REQUIRE(projectileParams.maxTargets == 1);

    REQUIRE(position.x == Catch::Approx(132.0f));
    REQUIRE(position.y == Catch::Approx(120.0f));

    REQUIRE(velocity.x == Catch::Approx(100.0f));
    REQUIRE(velocity.y == Catch::Approx(0.0f));

    REQUIRE(sprite.imagePath == "projectile_idle_1.png");
    REQUIRE(sprite.rect.size.x == Catch::Approx(10.0f));
    REQUIRE(sprite.rect.size.y == Catch::Approx(8.0f));

    REQUIRE(hitBox.offset.x == Catch::Approx(1.0f));
    REQUIRE(hitBox.offset.y == Catch::Approx(2.0f));
    REQUIRE(hitBox.size.x == Catch::Approx(8.0f));
    REQUIRE(hitBox.size.y == Catch::Approx(6.0f));

    const auto &playerAnimation = registry.getComponent<game::Animation>(player);

    REQUIRE(playerAnimation.state == game::AnimationState::Attack);
    REQUIRE(playerAnimation.direction == game::AnimationDirection::Right);
    REQUIRE(playerAnimation.stateTimeRemaining == Catch::Approx(0.4f));
}

TEST_CASE_METHOD(TestFixture, "InputSystem does not start attack while attack animation is active")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry, game::CharacterType::Melee);

    auto &animation = registry.getComponent<game::Animation>(player);
    animation.state = game::AnimationState::Attack;
    animation.direction = game::AnimationDirection::Right;
    animation.stateTimeRemaining = 0.5f;

    controller::InputState input{};
    input.mouseLeftPressed = true;
    input.mouseGrid = {200.0f, 100.0f};

    system.update(registry, config, input, 0.1f);

    requireNoPlayerAttacks(registry);

    const auto &animationAfter = registry.getComponent<game::Animation>(player);

    REQUIRE(animationAfter.state == game::AnimationState::Attack);
    REQUIRE(animationAfter.direction == game::AnimationDirection::Right);
    REQUIRE(animationAfter.stateTimeRemaining == Catch::Approx(0.4f));
}

TEST_CASE_METHOD(TestFixture, "InputSystem spawns unicorn special attack entity")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry, game::CharacterType::Ranged);

    controller::InputState input{};
    input.mouseRightPressed = true;
    input.mouseGrid = {200.0f, 108.0f};

    system.update(registry, config, input, 1.0f);

    const auto unicorns = registry.view<game::Damage, game::DamageTag, game::PlayerAttackTag, game::Position,
                                        game::Velocity, game::HitBox, view::Sprite, game::Animation>();

    REQUIRE(unicorns.size() == 1);

    const game::Entity unicorn = unicorns.front();

    const auto &damage = registry.getComponent<game::Damage>(unicorn);
    const auto &position = registry.getComponent<game::Position>(unicorn).p;
    const auto &velocity = registry.getComponent<game::Velocity>(unicorn).v;
    const auto &sprite = registry.getComponent<view::Sprite>(unicorn);
    const auto &hitBox = registry.getComponent<game::HitBox>(unicorn);
    const auto &animation = registry.getComponent<game::Animation>(unicorn);

    REQUIRE(damage.amount == Catch::Approx(-1.0f));
    REQUIRE(damage.pushBackForce == Catch::Approx(2.0f));
    REQUIRE(damage.stunChance == Catch::Approx(0.1f));
    REQUIRE(damage.kind == game::DamageKind::Unicorn);

    const auto &unicornParams = std::get<game::UnicornDamage>(damage.params);

    REQUIRE(unicornParams.speed == Catch::Approx(50.0f));

    REQUIRE(position.x == Catch::Approx(132.0f));
    REQUIRE(position.y == Catch::Approx(108.0f));

    REQUIRE(velocity.x == Catch::Approx(150.0f));
    REQUIRE(velocity.y == Catch::Approx(0.0f));

    REQUIRE(sprite.imagePath == "unicorn_walk_right_1.png");
    REQUIRE(sprite.rect.size.x == Catch::Approx(32.0f));
    REQUIRE(sprite.rect.size.y == Catch::Approx(32.0f));

    REQUIRE(hitBox.offset.x == Catch::Approx(1.0f));
    REQUIRE(hitBox.offset.y == Catch::Approx(2.0f));
    REQUIRE(hitBox.size.x == Catch::Approx(30.0f));
    REQUIRE(hitBox.size.y == Catch::Approx(30.0f));

    REQUIRE(animation.state == game::AnimationState::Walk);
    REQUIRE(animation.direction == game::AnimationDirection::Right);
    REQUIRE(animation.currentFrame == 0);
    REQUIRE(animation.frameTimer == Catch::Approx(0.0f));

    const auto &playerAnimation = registry.getComponent<game::Animation>(player);

    REQUIRE(playerAnimation.state == game::AnimationState::Attack);
    REQUIRE(playerAnimation.direction == game::AnimationDirection::Right);
    REQUIRE(playerAnimation.stateTimeRemaining == Catch::Approx(0.4f));

    const auto &cooldown = registry.getComponent<game::PlayerAttackCooldown>(player);

    REQUIRE(cooldown.attackDurationSec == Catch::Approx(0.5f));
    REQUIRE(cooldown.attackRemainingSec == Catch::Approx(0.0f));
    REQUIRE(cooldown.specialAttackDurationSec == Catch::Approx(0.5f));
    REQUIRE(cooldown.specialAttackRemainingSec == Catch::Approx(0.5f));
}

TEST_CASE_METHOD(TestFixture, "InputSystem spawns melee area special attack entity")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry, game::CharacterType::Melee);

    controller::InputState input{};
    input.mouseRightPressed = true;
    input.mouseGrid = {200.0f, 100.0f};

    system.update(registry, config, input, 1.0f);

    const auto areas = registry.view<game::Damage, game::DamageTag, game::PlayerAttackTag, game::Position, game::HitBox,
                                     view::Sprite, game::Animation>();

    REQUIRE(areas.size() == 1);

    const game::Entity area = areas.front();

    const auto &damage = registry.getComponent<game::Damage>(area);
    const auto &position = registry.getComponent<game::Position>(area).p;
    const auto &hitBox = registry.getComponent<game::HitBox>(area);
    const auto &sprite = registry.getComponent<view::Sprite>(area);
    const auto &animation = registry.getComponent<game::Animation>(area);

    REQUIRE(damage.amount == Catch::Approx(-0.95f));
    REQUIRE(damage.pushBackForce == Catch::Approx(3.0f));
    REQUIRE(damage.stunChance == Catch::Approx(0.25f));
    REQUIRE(damage.kind == game::DamageKind::Area);

    const auto &areaParams = std::get<game::AreaDamage>(damage.params);

    REQUIRE(areaParams.radius == Catch::Approx(48.0f));
    REQUIRE(areaParams.activeTimeSec == Catch::Approx(1.2f));
    REQUIRE(areaParams.elapsedSec == Catch::Approx(0.0f));
    REQUIRE(areaParams.initialHit == Catch::Approx(0.5f));
    REQUIRE(areaParams.damageTicks == 3);
    REQUIRE(areaParams.elapsedSecSinceLastTick == Catch::Approx(0.0f));

    REQUIRE(position.x == Catch::Approx(100.0f));
    REQUIRE(position.y == Catch::Approx(100.0f));

    REQUIRE(sprite.imagePath == "melee_area_idle_1.png");
    REQUIRE(sprite.rect.size.x == Catch::Approx(32.0f));
    REQUIRE(sprite.rect.size.y == Catch::Approx(32.0f));

    REQUIRE(hitBox.offset.x == Catch::Approx(1.0f));
    REQUIRE(hitBox.offset.y == Catch::Approx(2.0f));
    REQUIRE(hitBox.size.x == Catch::Approx(30.0f));
    REQUIRE(hitBox.size.y == Catch::Approx(30.0f));

    REQUIRE(animation.state == game::AnimationState::Idle);
    REQUIRE(animation.direction == game::AnimationDirection::None);
    REQUIRE(animation.currentFrame == 0);
    REQUIRE(animation.frameTimer == Catch::Approx(0.0f));

    REQUIRE_FALSE(registry.hasComponent<game::Velocity>(area));
    REQUIRE(registry.hasComponent<game::Sound>(player));

    const auto &playerAnimation = registry.getComponent<game::Animation>(player);

    REQUIRE(playerAnimation.state == game::AnimationState::Attack);
    REQUIRE(playerAnimation.direction == game::AnimationDirection::Right);
    REQUIRE(playerAnimation.stateTimeRemaining == Catch::Approx(0.4f));

    const auto &cooldown = registry.getComponent<game::PlayerAttackCooldown>(player);

    REQUIRE(cooldown.attackDurationSec == Catch::Approx(0.5f));
    REQUIRE(cooldown.attackRemainingSec == Catch::Approx(0.0f));
    REQUIRE(cooldown.specialAttackDurationSec == Catch::Approx(0.5f));
    REQUIRE(cooldown.specialAttackRemainingSec == Catch::Approx(0.5f));
}

TEST_CASE_METHOD(TestFixture, "InputSystem starts melee special attack animation towards the mouse")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry, game::CharacterType::Melee);

    controller::InputState input{};
    input.mouseRightPressed = true;
    input.mouseGrid = {50.0f, 100.0f};

    system.update(registry, config, input, 1.0f);

    const game::Entity attack = requireSinglePlayerAttack(registry);
    const auto &damage = registry.getComponent<game::Damage>(attack);

    REQUIRE(damage.kind == game::DamageKind::Area);

    const auto &playerAnimation = registry.getComponent<game::Animation>(player);

    REQUIRE(playerAnimation.state == game::AnimationState::Attack);
    REQUIRE(playerAnimation.direction == game::AnimationDirection::Left);
}

TEST_CASE_METHOD(TestFixture, "InputSystem updates attack cooldowns and clamps them to zero")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry, game::CharacterType::Ranged);

    auto &cooldownBefore = registry.getComponent<game::PlayerAttackCooldown>(player);
    cooldownBefore.attackRemainingSec = 0.4f;
    cooldownBefore.specialAttackRemainingSec = 0.2f;

    controller::InputState input{};

    system.update(registry, config, input, 0.3f);

    const auto &cooldown = registry.getComponent<game::PlayerAttackCooldown>(player);

    REQUIRE(cooldown.attackDurationSec == Catch::Approx(0.5f));
    REQUIRE(cooldown.attackRemainingSec == Catch::Approx(0.1f));
    REQUIRE(cooldown.specialAttackDurationSec == Catch::Approx(0.5f));
    REQUIRE(cooldown.specialAttackRemainingSec == Catch::Approx(0.0f));
}

TEST_CASE_METHOD(TestFixture, "InputSystem blocks normal attack while normal attack cooldown is active")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry, game::CharacterType::Ranged);

    setAttackCooldowns(registry, player, 0.5f, 0.0f);

    controller::InputState input{};
    input.mouseLeftPressed = true;
    input.mouseGrid = {200.0f, 120.0f};

    system.update(registry, config, input, 0.0f);

    requireNoPlayerAttacks(registry);

    const auto &cooldown = registry.getComponent<game::PlayerAttackCooldown>(player);

    REQUIRE(cooldown.attackRemainingSec == Catch::Approx(0.5f));
    REQUIRE(cooldown.specialAttackRemainingSec == Catch::Approx(0.0f));
}

TEST_CASE_METHOD(TestFixture, "InputSystem blocks special attack while special attack cooldown is active")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry, game::CharacterType::Ranged);

    setAttackCooldowns(registry, player, 0.0f, 0.5f);

    controller::InputState input{};
    input.mouseRightPressed = true;
    input.mouseGrid = {200.0f, 108.0f};

    system.update(registry, config, input, 0.0f);

    requireNoPlayerAttacks(registry);

    const auto &cooldown = registry.getComponent<game::PlayerAttackCooldown>(player);

    REQUIRE(cooldown.attackRemainingSec == Catch::Approx(0.0f));
    REQUIRE(cooldown.specialAttackRemainingSec == Catch::Approx(0.5f));
}

TEST_CASE_METHOD(TestFixture, "InputSystem does not block normal ranged attack with active special cooldown")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry, game::CharacterType::Ranged);

    setAttackCooldowns(registry, player, 0.0f, 0.5f);

    controller::InputState input{};
    input.mouseLeftPressed = true;
    input.mouseGrid = {200.0f, 120.0f};

    system.update(registry, config, input, 0.0f);

    const game::Entity attack = requireSinglePlayerAttack(registry);
    const auto &damage = registry.getComponent<game::Damage>(attack);

    REQUIRE(damage.kind == game::DamageKind::Projectile);
    REQUIRE_FALSE(registry.hasComponent<game::Animation>(attack));

    const auto &cooldown = registry.getComponent<game::PlayerAttackCooldown>(player);

    REQUIRE(cooldown.attackRemainingSec == Catch::Approx(0.5f));
    REQUIRE(cooldown.specialAttackRemainingSec == Catch::Approx(0.5f));
}

TEST_CASE_METHOD(TestFixture, "InputSystem does not block special ranged attack with active normal cooldown")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry, game::CharacterType::Ranged);

    setAttackCooldowns(registry, player, 0.5f, 0.0f);

    controller::InputState input{};
    input.mouseRightPressed = true;
    input.mouseGrid = {200.0f, 108.0f};

    system.update(registry, config, input, 0.0f);

    const game::Entity attack = requireSinglePlayerAttack(registry);
    const auto &damage = registry.getComponent<game::Damage>(attack);

    REQUIRE(damage.kind == game::DamageKind::Unicorn);
    REQUIRE(registry.hasComponent<game::Animation>(attack));

    const auto &cooldown = registry.getComponent<game::PlayerAttackCooldown>(player);

    REQUIRE(cooldown.attackRemainingSec == Catch::Approx(0.5f));
    REQUIRE(cooldown.specialAttackRemainingSec == Catch::Approx(0.5f));
}
TEST_CASE_METHOD(TestFixture, "InputSystem does not block normal melee attack with active special cooldown")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry, game::CharacterType::Melee);

    setAttackCooldowns(registry, player, 0.0f, 0.5f);

    controller::InputState input{};
    input.mouseLeftPressed = true;
    input.mouseGrid = {200.0f, 100.0f};

    system.update(registry, config, input, 0.0f);

    const game::Entity attack = requireSinglePlayerAttack(registry);
    const auto &damage = registry.getComponent<game::Damage>(attack);

    REQUIRE(damage.kind == game::DamageKind::MeleeArc);
    REQUIRE_FALSE(registry.hasComponent<view::Sprite>(attack));
    REQUIRE_FALSE(registry.hasComponent<game::Animation>(attack));

    const auto &cooldown = registry.getComponent<game::PlayerAttackCooldown>(player);

    REQUIRE(cooldown.attackRemainingSec == Catch::Approx(0.5f));
    REQUIRE(cooldown.specialAttackRemainingSec == Catch::Approx(0.5f));
}

TEST_CASE_METHOD(TestFixture, "InputSystem does not block special melee attack with active normal cooldown")
{
    game::Registry registry;
    game::InputSystem system;
    const config::GameConfig config = makeInputSystemTestConfig();

    const game::Entity player = addPlayer(registry, game::CharacterType::Melee);

    setAttackCooldowns(registry, player, 0.5f, 0.0f);

    controller::InputState input{};
    input.mouseRightPressed = true;
    input.mouseGrid = {200.0f, 100.0f};

    system.update(registry, config, input, 0.0f);

    const game::Entity attack = requireSinglePlayerAttack(registry);
    const auto &damage = registry.getComponent<game::Damage>(attack);

    REQUIRE(damage.kind == game::DamageKind::Area);
    REQUIRE(registry.hasComponent<view::Sprite>(attack));
    REQUIRE(registry.hasComponent<game::Animation>(attack));

    const auto &cooldown = registry.getComponent<game::PlayerAttackCooldown>(player);

    REQUIRE(cooldown.attackRemainingSec == Catch::Approx(0.5f));
    REQUIRE(cooldown.specialAttackRemainingSec == Catch::Approx(0.5f));
}
