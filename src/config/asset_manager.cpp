#include "config/asset_manager.hpp"
#include "logging/log.hpp"
#include <format>

namespace config {

AnimationFrame AssetManager::getAnimationFrame(const std::string &entityName, const AnimationConfig &animationConfig,
                                               const game::AnimationState state,
                                               const game::AnimationDirection direction, const size_t frameNum,
                                               const SpriteConfig &fallback)
{
    if (!animationConfig.stateToDirection.contains(state)) {
        logger::log(logger::ERROR,
                    std::format("No animation state {} found for entity {}.", toString(state), entityName));
        return {fallback, 0.16f, 1};
    }

    const DirectionalAnimationConfig &stateConfig = animationConfig.stateToDirection.at(state);

    if (!stateConfig.directionToFrames.contains(direction)) {
        logger::log(logger::ERROR, std::format("No animation direction {} found for state {} and entity {}.",
                                               toString(direction), toString(state), entityName));
        return {fallback, stateConfig.frameDuration, 1};
    }

    const std::vector<SpriteConfig> &frames = stateConfig.directionToFrames.at(direction);

    if (frames.empty()) {
        logger::log(logger::ERROR, std::format("No animation frames found for state {}, direction {} and entity {}.",
                                               toString(state), toString(direction), entityName));
        return {fallback, stateConfig.frameDuration, 1};
    }

    if (frameNum >= frames.size()) {
        logger::log(logger::ERROR,
                    std::format("Frame number {} out of bounds for state {}, direction {} and entity {}.", frameNum,
                                toString(state), toString(direction), entityName));
        return {fallback, stateConfig.frameDuration, 1};
    }

    return {frames.at(frameNum), stateConfig.frameDuration, frames.size()};
}

AnimationFrame config::AssetManager::getPlayerAnimationFrame(const GameConfig &config,
                                                             const game::CharacterType characterType,
                                                             const game::AnimationState state,
                                                             const game::AnimationDirection direction,
                                                             const size_t frameNum)
{
    const config::PlayerClassConfig &classConfig = config.playerClasses.getByType(characterType);
    const std::string entityName = std::format("player: {}", toString(characterType));
    return getAnimationFrame(entityName, classConfig.animations, state, direction, frameNum, config.fallbackSprite);
}

AnimationFrame config::AssetManager::getProjectileAnimationFrame(const GameConfig &config,
                                                                 const ProjectileAttackConfig &projectileConfig,
                                                                 const game::AnimationState state,
                                                                 const game::AnimationDirection direction,
                                                                 const size_t frameNum)
{
    return getAnimationFrame("projectile", projectileConfig.animations, state, direction, frameNum,
                             config.fallbackSprite);
}

} // namespace config