#include "config/animation_config_helper.hpp"
#include "logging/log.hpp"
#include <format>

namespace config {

AnimationFrame AnimationConfigHelper::getAnimationFrame(const std::string &entityName,
                                                        const AnimationConfig &animationConfig,
                                                        const game::AnimationState state,
                                                        const game::AnimationDirection direction, const size_t frameNum,
                                                        const SpriteConfig &fallback)
{
    AnimationStateConfig defaultStateConfig;
    AnimationFrame fallbackFrame{
        .spriteConfig = fallback,
        .totalFrames = 1,
        .frameDuration = defaultStateConfig.frameDuration,
        .moveSpeedMultiplier = defaultStateConfig.moveSpeedMultiplier,
    };

    if (!animationConfig.stateToStateConfig.contains(state)) {
        logger::log(logger::ERROR,
                    std::format("No animation state {} found for entity {}.", toString(state), entityName));
        return fallbackFrame;
    }

    const AnimationStateConfig &stateConfig = animationConfig.stateToStateConfig.at(state);

    if (!stateConfig.directionToFrames.contains(direction)) {
        logger::log(logger::ERROR, std::format("No animation direction {} found for state {} and entity {}.",
                                               toString(direction), toString(state), entityName));
        return fallbackFrame;
    }

    const std::vector<SpriteConfig> &frames = stateConfig.directionToFrames.at(direction);

    if (frames.empty()) {
        logger::log(logger::ERROR, std::format("No animation frames found for state {}, direction {} and entity {}.",
                                               toString(state), toString(direction), entityName));
        return fallbackFrame;
    }

    if (frameNum >= frames.size()) {
        logger::log(logger::ERROR,
                    std::format("Frame number {} out of bounds for state {}, direction {} and entity {}.", frameNum,
                                toString(state), toString(direction), entityName));
        return fallbackFrame;
    }

    return AnimationFrame{
        .spriteConfig = frames.at(frameNum),
        .totalFrames = frames.size(),
        .frameDuration = stateConfig.frameDuration,
        .moveSpeedMultiplier = stateConfig.moveSpeedMultiplier,
    };
}

AnimationFrame config::AnimationConfigHelper::getPlayerAnimationFrame(const GameConfig &config,
                                                                      const game::CharacterType characterType,
                                                                      const game::AnimationState state,
                                                                      const game::AnimationDirection direction,
                                                                      const size_t frameNum)
{
    const config::PlayerClassConfig &classConfig = config.playerClasses.getByType(characterType);
    const std::string entityName = std::format("player: {}", toString(characterType));
    return getAnimationFrame(entityName, classConfig.animations, state, direction, frameNum, config.fallbackSprite);
}

AnimationFrame config::AnimationConfigHelper::getEnemyAnimationFrame(const GameConfig &config,
                                                                     const game::EnemyType enemyType,
                                                                     const game::AnimationState state,
                                                                     const game::AnimationDirection direction,
                                                                     const size_t frameNum)
{
    const config::EnemyClassConfig &classConfig = config.enemyClasses.getByType(enemyType);
    const std::string entityName = std::format("enemy: {}", toString(enemyType));
    return getAnimationFrame(entityName, classConfig.animations, state, direction, frameNum, config.fallbackSprite);
}

AnimationFrame config::AnimationConfigHelper::getProjectileAnimationFrame(
    const GameConfig &config, const ProjectileAttackConfig &projectileConfig, const game::AnimationState state,
    const game::AnimationDirection direction, const size_t frameNum)
{
    return getAnimationFrame("projectile", projectileConfig.animations, state, direction, frameNum,
                             config.fallbackSprite);
}

} // namespace config
