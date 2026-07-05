#pragma once
#include "game_config.hpp"

namespace config {

struct AnimationFrame {
    const SpriteConfig &spriteConfig;
    size_t totalFrames;
    float frameDuration;
    float moveSpeedMultiplier;
};

class AnimationConfigHelper {
    static AnimationFrame getAnimationFrame(const std::string &entityName, const AnimationConfig &animationConfig,
                                            const game::AnimationState state, const game::AnimationDirection direction,
                                            const size_t frameNum, const SpriteConfig &fallback);

  public:
    AnimationConfigHelper() = delete;
    ~AnimationConfigHelper() = delete;

    static AnimationFrame getPlayerAnimationFrame(const GameConfig &config, const game::CharacterType characterType,
                                                  const game::AnimationState state,
                                                  const game::AnimationDirection direction, const size_t frameNum);

    static AnimationFrame getEnemyAnimationFrame(const GameConfig &config, const game::EnemyType enemyType,
                                                 const game::AnimationState state,
                                                 const game::AnimationDirection direction, const size_t frameNum);

    static AnimationFrame getProjectileAnimationFrame(const GameConfig &config,
                                                      const ProjectileAttackConfig &projectileConfig,
                                                      const game::AnimationState state,
                                                      const game::AnimationDirection direction, const size_t frameNum);

    static AnimationFrame getUnicornAnimationFrame(const GameConfig &config, const UnicornAttackConfig &unicornConfig,
                                                   const game::AnimationState state,
                                                   const game::AnimationDirection direction, const size_t frameNum);

    static AnimationFrame getAreaAnimationFrame(const GameConfig &config, const AreaAttackConfig &areaConfig,
                                                const game::AnimationState state,
                                                const game::AnimationDirection direction, const size_t frameNum);
};

} // namespace config