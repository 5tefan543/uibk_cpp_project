#pragma once

#include "audio/audio_cache.hpp"
#include "audio/audio_controller.hpp"
#include "game/game.hpp"
#include "input/input_state.hpp"
#include "state/state_manager.hpp"

namespace controller {

class Controller {
  private:
    audio::AudioCache audioCache_;
    audio::AudioController audioController_;
    StateManager stateManager_;

  public:
    Controller();
    ~Controller();
    void update(const InputState &input, float dt);
    BaseState &getCurrentState();
};

} // namespace controller