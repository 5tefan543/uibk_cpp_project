#pragma once
#include "audio/audio_controller.hpp"
#include "game/ecs/registry.hpp"
namespace game {

class SoundSystem {
  private:
    audio::AudioController manager_;

  public:
    void update(Registry &registry);
};
} // namespace game
