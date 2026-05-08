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

    static void saveGame(const controller::PersistedGame &persistedGame);
    static void loadGame(controller::PersistedGame &persistedGame);
    static bool hasSavedGame();
    static void deleteSave();

    static void storeLeaderboardEntry(const std::string &playerName, int score);
    static std::vector<std::pair<std::string, int>> getTopNLeaderboardEntries(int topN);

    static void saveConfig(const controller::GameConfig &config);
    static controller::GameConfig loadConfig();

  private:
    static std::optional<controller::GameConfig> configCache_;
};

} // namespace controller