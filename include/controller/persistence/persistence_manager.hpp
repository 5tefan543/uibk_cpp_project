#pragma once
#include "controller/persistence/config_game.hpp"
#include "controller/persistence/persisted_game.hpp"
#include "controller/state/state_manager.hpp"
#include "game/ecs/registry.hpp"
namespace controller {

class PersistenceManager {
  public:
    PersistenceManager();
    ~PersistenceManager();

    void saveGame(const game::Registry &registry);
    void loadGame(game::Registry &registry);

    void saveConfig(const controller::GameConfig &config);
    controller::GameConfig loadConfig();
};

} // namespace controller