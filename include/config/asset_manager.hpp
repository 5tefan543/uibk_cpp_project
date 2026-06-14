#pragma once
#include "game_config.hpp"

namespace config {

class AssetManager {
    static const SpriteConfig &getSpriteConfig(const std::string &entityName, const AnimationConfig &animationConfig,
                                               const game::AnimationState &state,
                                               const game::AnimationDirection &direction, const size_t frameNum);

  public:
    AssetManager() = delete;
    ~AssetManager() = delete;

    static const SpriteConfig &getPlayerSpriteConfig(const GameConfig &config, const game::CharacterType &characterType,
                                                     const game::AnimationState &state,
                                                     const game::AnimationDirection &direction, const size_t frameNum);
};

} // namespace config