#pragma once
#include "audio/audio_manager.hpp"
#include "game/ecs/registry.hpp"
namespace game {

class SoundSystem {
  private:
    audio::AudioManager manager_;

  public:
    void update(Registry &registry);
};
} // namespace game
