#include "game/ecs/systems/enemy_ai_system.hpp"
#include "config/animation_config_helper.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/location_table.hpp"
#include <cmath>

namespace game {

namespace {

void setAnimationState(Animation &animation, AnimationState state, AnimationDirection direction)
{
    if (animation.state == state && animation.direction == direction) {
        return;
    }

    animation.state = state;
    animation.direction = direction;
    animation.currentFrame = 0;
    animation.frameTimer = 0.0f;
}

} // namespace

const float minDistanceEnemyPlayer = 5;
const float enemyRepelRadius = 50;
const float enemyRepelProximityRampParam = 1.5;
const float enemyRelSpeedCutoffPercentage = 0.02;

void EnemyAI::update(Registry &registry, const config::GameConfig &config, LocationTable &locationTable, float dt)
{
    const auto players = registry.view<PlayerTag, Position>();
    if (players.empty()) {
        return;
    }
    const auto player = players.front();
    const Position &playerPos = registry.getComponent<Position>(player);

    for (auto enemy : registry.view<EnemyTag, Velocity, EnemyStats, Position, Animation>()) {
        updateEnemyVelocityTowardsPlayer(registry, locationTable, playerPos, enemy);
        updateEnemyAnimationState(registry, enemy, dt);
        // attack methods similar as in input system
        applyAnimationMoveSpeedModifier(registry, config, enemy);
    }
}

void EnemyAI::updateEnemyVelocityTowardsPlayer(Registry &registry, LocationTable &locationTable,
                                               const Position &playerPos, Entity enemy)
{
    const Vec2 playerPosVec{playerPos.x, playerPos.y};

    auto &[vx, vy] = registry.getComponent<Velocity>(enemy);
    auto &[px, py] = registry.getComponent<Position>(enemy);

    EnemyStats &enemyStats = registry.getComponent<EnemyStats>(enemy);
    if (enemyStats.moveSpeed == 0) {
        vx = 0;
        vy = 0;
        return;
    }
    Vec2 enemyPosVec{px, py};

    // Set movement direction exactly towards player
    Vec2 v = playerPosVec - enemyPosVec;

    // TODO: add player attack: -> stwa: maybe not in this method and similar to input system ?
    // if (v.length() < 30) {attack_player();}

    // Prevent shooting over target (player) position
    if (v.length() < minDistanceEnemyPlayer) {
        vx = 0; // TODO: into Vec2
        vy = 0; // TODO: into Vec2
        return;
    }

    // Calc. repelling force between enemies
    auto enemiesInRange = locationTable.getEntitiesInRange(enemyPosVec, enemyRepelRadius, registry);
    Vec2<float> repelOffset = {0, 0};
    for (auto [e, position] : enemiesInRange) {
        if (e == enemy) {
            continue;
        }
        const auto p = Vec2{position.x, position.y}; // TODO: into Vec2
        const auto pToOther = (enemyPosVec - p);
        const auto b = (std::pow(pToOther.length(), enemyRepelProximityRampParam));
        if (b != 0) {
            repelOffset += pToOther / b; // increase repelling with proximity
        }
    }

    // Divert straight forwards movement to player with repelling offset
    v.normalize();
    v += repelOffset;
    v *= enemyStats.moveSpeed;

    // Enforce enemy speed limit if repelling force would boost it over max
    auto l = std::min(v.length(), enemyStats.moveSpeed);

    // Prevents jittery movement in enemy heaps by
    // - decreasing speed inverse propotional to max speed
    // - and stopping movement below a certaing percentage
    l *= std::pow(l / enemyStats.moveSpeed, 2);
    if (l / enemyStats.moveSpeed < enemyRelSpeedCutoffPercentage) {
        vx = 0; // TODO: into Vec2
        vy = 0; // TODO: into Vec2
        return;
    }
    v.setLength(l);

    vx = v.x; // TODO: into Vec2
    vy = v.y; // TODO: into Vec2
}

void EnemyAI::updateEnemyAnimationState(Registry &registry, Entity enemy, float dt)
{
    Animation &animation = registry.getComponent<Animation>(enemy);
    const Velocity &velocity = registry.getComponent<Velocity>(enemy);

    if (animation.stateTimeRemaining > 0.0f) {
        animation.stateTimeRemaining = std::max(0.0f, animation.stateTimeRemaining - dt);
    }

    if (animation.stateTimeRemaining > 0.0f) {
        return;
    }

    const bool isMoving = std::abs(velocity.x) > 0.1f || std::abs(velocity.y) > 0.1f;
    const AnimationState nextState = isMoving ? AnimationState::Walk : AnimationState::Idle;

    AnimationDirection direction = animation.direction;

    const auto absVelocity = Vec2{velocity.x, velocity.y}.abs();
    bool isHorizMove = absVelocity.x >= absVelocity.y;

    if (isHorizMove && absVelocity.x > 0.1f) {
        direction = velocity.x > 0.0f ? AnimationDirection::Right : AnimationDirection::Left;
    }

    setAnimationState(animation, nextState, direction);
}

void EnemyAI::applyAnimationMoveSpeedModifier(Registry &registry, const config::GameConfig &config, Entity enemyEntity)
{
    const Animation &enemyAnimation = registry.getComponent<Animation>(enemyEntity);
    const EnemyStats &enemyStats = registry.getComponent<EnemyStats>(enemyEntity);
    Velocity &enemyVelocity = registry.getComponent<Velocity>(enemyEntity);

    const config::AnimationFrame currentFrame = config::AnimationConfigHelper::getEnemyAnimationFrame(
        config, enemyStats.enemyType, enemyAnimation.state, enemyAnimation.direction, enemyAnimation.currentFrame);

    enemyVelocity.x *= currentFrame.moveSpeedMultiplier;
    enemyVelocity.y *= currentFrame.moveSpeedMultiplier;
}

} // namespace game