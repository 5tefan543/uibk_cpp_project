#pragma once
#include "game_config.hpp"

namespace config {

struct AnimationFrame {
    const SpriteConfig &spriteConfig;
    float frameDuration;
    size_t totalFrames;
};

class AssetManager {
    static AnimationFrame getAnimationFrame(const std::string &entityName, const AnimationConfig &animationConfig,
                                            const game::AnimationState state, const game::AnimationDirection direction,
                                            const size_t frameNum, const SpriteConfig &fallback);

  public:
    AssetManager() = delete;
    ~AssetManager() = delete;

    static AnimationFrame getPlayerAnimationFrame(const GameConfig &config, const game::CharacterType characterType,
                                                  const game::AnimationState state,
                                                  const game::AnimationDirection direction, const size_t frameNum);

    static AnimationFrame getProjectileAnimationFrame(const GameConfig &config,
                                                      const ProjectileAttackConfig &projectileConfig,
                                                      const game::AnimationState state,
                                                      const game::AnimationDirection direction, const size_t frameNum);
};

} // namespace config