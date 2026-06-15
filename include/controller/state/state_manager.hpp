#pragma once

#include <memory>
#include <vector>

#include "audio/audio_controller.hpp"
#include "controller/state/state.hpp"

namespace controller {

class StateManager {
  private:
    std::vector<std::unique_ptr<BaseState>> states_;
    audio::AudioController *audioController_ = nullptr;

  public:
    explicit StateManager(audio::AudioController *audioController = nullptr);
    void push(std::unique_ptr<BaseState> state);
    void pop();
    BaseState &getCurrent();
    bool isEmpty() const;
    void clear();
    void replaceCurrent(std::unique_ptr<BaseState> state);
    void applyAction(StateTransitionAction action);
    std::string getDebugInfo() const;
    void printDebugInfo() const;
};

} // namespace controller