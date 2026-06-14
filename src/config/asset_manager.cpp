#include "config/asset_manager.hpp"
#include "logging/log.hpp"
#include <format>

namespace config {

const SpriteConfig &AssetManager::getSpriteConfig(const std::string &entityName, const AnimationConfig &animationConfig,
                                                  const game::AnimationState &state,
                                                  const game::AnimationDirection &direction, const size_t frameNum)
{
    if (!animationConfig.stateToDirection.contains(state)) {
        logger::log(logger::ERROR,
                    std::format("No animation state {} found for entity {}.", toString(state), entityName));
        return animationConfig.fallbackFrame;
    }

    const DirectionalAnimationConfig &stateConfig = animationConfig.stateToDirection.at(state);

    if (!stateConfig.directionToFrames.contains(direction)) {
        logger::log(logger::ERROR, std::format("No animation direction {} found for state {} and entity {}.",
                                               toString(direction), toString(state), entityName));
        return animationConfig.fallbackFrame;
    }

    const std::vector<SpriteConfig> &frames = stateConfig.directionToFrames.at(direction);

    if (frames.empty()) {
        logger::log(logger::ERROR, std::format("No animation frames found for state {}, direction {} and entity {}.",
                                               toString(state), toString(direction), entityName));
        return animationConfig.fallbackFrame;
    }

    if (frameNum >= frames.size()) {
        logger::log(logger::ERROR,
                    std::format("Frame number {} out of bounds for state {}, direction {} and entity {}.", frameNum,
                                toString(state), toString(direction), entityName));
        return animationConfig.fallbackFrame;
    }

    return frames.at(frameNum);
}

const SpriteConfig &config::AssetManager::getPlayerSpriteConfig(const GameConfig &config,
                                                                const game::CharacterType &characterType,
                                                                const game::AnimationState &state,
                                                                const game::AnimationDirection &direction,
                                                                const size_t frameNum)
{
    const config::PlayerClassConfig &classConfig = config.playerClasses.getByType(characterType);
    const std::string entityName = std::format("player: {}", toString(characterType));
    return getSpriteConfig(entityName, classConfig.animations, state, direction, frameNum);
}

} // namespace config