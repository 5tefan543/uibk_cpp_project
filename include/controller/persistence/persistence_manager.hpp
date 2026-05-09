#pragma once
#include "config_game.hpp"
#include "persisted_game.hpp"
#include <optional>
#include <string>
#include <vector>
namespace controller {

class PersistenceManager {
  public:
    PersistenceManager() = delete;
    ~PersistenceManager() = delete;

    static bool saveGame(const PersistedGame &persistedGame);
    static void loadGame(PersistedGame &persistedGame);
    static bool hasSavedGame();
    static void deleteSave();

    static bool storeLeaderboardEntry(const std::string &playerName, int score);
    static std::vector<std::pair<std::string, int>> getTopNLeaderboardEntries(int topN);

    static bool saveConfig(const GameConfig &config);
    static GameConfig loadConfig();
};

} // namespace controller