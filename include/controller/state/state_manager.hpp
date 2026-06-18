#pragma once

#include "audio/audio_cache.hpp"
#include <memory>
#include <vector>

#include "audio/audio_controller.hpp"
#include "config/game_config.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "controller/state/state.hpp"

namespace controller {
class StateManager {
  private:
    const config::GameConfig &config_ = controller::PersistenceManager::getConfig();
    std::vector<std::unique_ptr<BaseState>> states_;
    audio::AudioController audioController_;

  public:
    void push(std::unique_ptr<BaseState> state);
    void pop();
    BaseState &getCurrent();
    bool isEmpty() const;
    void clear();
    void replaceCurrent(std::unique_ptr<BaseState> state);
    void updateAudio();
    void applyAction(StateTransitionAction action);
    std::string getDebugInfo() const;
    void printDebugInfo() const;
};

} // namespace controller