#pragma once
#include "persisted_game.hpp"
#include "config_game.hpp"
#include <vector>
#include <string>
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
    std::vector<std::pair<std::string, int>> getTopNLeaderboardEntries(int topN);

    void saveConfig(const controller::GameConfig &config);
    controller::GameConfig loadConfig();
};

} // namespace controller