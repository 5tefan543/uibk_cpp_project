#pragma once
#include "audio/audio_controller.hpp"
#include "game/ecs/registry.hpp"
namespace game {

class SoundSystem {
  private:
    audio::AudioCache cache_;
    audio::AudioController manager_;

  public:
    SoundSystem() : manager_(cache_) {}

    void update(Registry &registry);
};
} // namespace game
