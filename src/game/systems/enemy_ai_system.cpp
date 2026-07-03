#include "game/ecs/systems/enemy_ai_system.hpp"
#include "config/animation_config_helper.hpp"
#include "config/game_config.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/location_table.hpp"
#include <cmath>

namespace game {

const float minDistanceEnemyPlayer = 5;
const float enemyRepelRadius = 50;
const float enemyRepelProximityRampParam = 1.5;
const float enemyRelSpeedCutoffPercentage = 0.02;

void EnemyAI::update(Registry &registry, const config::GameConfig &config, LocationTable &locationTable, float dtSec)
{
    const auto players = registry.view<PlayerTag, Position>();
    if (players.empty()) {
        return;
    }
    const auto player = players.front();
    const Position &playerPos = registry.getComponent<Position>(player);

    for (auto enemy : registry.view<EnemyTag, Velocity, EnemyStats, Position, Animation>()) {
        updateEnemyVelocityTowardsPlayer(registry, locationTable, playerPos, enemy);
        updateEnemyAnimationState(registry, enemy, dtSec);
        applyAnimationMoveSpeedModifier(registry, config, enemy);
    }
}

void EnemyAI::updateEnemyVelocityTowardsPlayer(Registry &registry, LocationTable &locationTable,
                                               const Position &playerPosition, Entity enemy)
{
    using geometry::Vec2;

    const float minDistanceEnemyPlayer = 5;
    const float enemyRepelRadius = 50;
    const float enemyRepelProximityRampParam = 1.5;
    const float enemyRelSpeedCutoffPercentage = 0.02;

    const Vec2<float> &playerPos = playerPosition.p;

    Vec2<float> &v = registry.getComponent<Velocity>(enemy).v;
    const Vec2<float> &enemyPos = registry.getComponent<Position>(enemy).p;

    EnemyStats &enemyStats = registry.getComponent<EnemyStats>(enemy);
    if (enemyStats.moveSpeed == 0) {
        v = {0, 0};
        return;
    }

    // Set movement direction exactly towards player
    v = playerPos - enemyPos;

    // TODO: add player attack: -> stwa: maybe not in this method and similar to input system ?
    // if (v.length() < 30) {attack_player();}

    // Prevent shooting over target (player) position
    if (v.length() < minDistanceEnemyPlayer) {
        v = {0, 0};
        return;
    }

    // Calc. repelling force between enemies
    auto enemiesInRange = locationTable.getEntitiesInRange(enemyPos, enemyRepelRadius, registry);
    Vec2<float> repelOffset = {0, 0};
    for (const auto &[otherEnemy, otherPos] : enemiesInRange) {
        if (otherEnemy == enemy) {
            continue;
        }
        const auto pToOther = (enemyPos - otherPos.p);
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
        v = {0, 0};
        return;
    }
    v.setLength(l);
}

void EnemyAI::updateEnemyAnimationState(Registry &registry, Entity enemy, float dtSec)
{
    using geometry::Vec2;

    Animation &animation = registry.getComponent<Animation>(enemy);
    const auto &velocity = registry.getComponent<Velocity>(enemy).v;

    if (animation.stateTimeRemaining > 0.0f) {
        animation.stateTimeRemaining = std::max(0.0f, animation.stateTimeRemaining - dtSec);
    }

    if (animation.stateTimeRemaining > 0.0f) {
        return;
    }

    const auto absVelocity = velocity.abs();
    const bool isMoving = (absVelocity > Vec2{0.1f, 0.1f}).some();
    const AnimationState nextState = isMoving ? AnimationState::Walk : AnimationState::Idle;

    AnimationDirection direction = animation.direction;

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
    auto &enemyVelocity = registry.getComponent<Velocity>(enemyEntity).v;

    const config::AnimationFrame currentFrame = config::AnimationConfigHelper::getEnemyAnimationFrame(
        config, enemyStats.enemyType, enemyAnimation.state, enemyAnimation.direction, enemyAnimation.currentFrame);

    enemyVelocity *= currentFrame.moveSpeedMultiplier;
}

} // namespace game