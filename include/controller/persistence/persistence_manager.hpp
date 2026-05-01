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

    void saveGame(const controller::PersistedGame &persistedGame);
    void loadGame(controller::PersistedGame &persistedGame);
    bool hasSavedGame();
    void deleteSave();

    void storeLeaderboardEntry(const std::string &playerName, int score);
    std::vector<std::pair<std::string, int>> getLeaderboardEntries(int topN);

    void saveConfig(const controller::GameConfig &config);
    controller::GameConfig loadConfig();
};

} // namespace controller