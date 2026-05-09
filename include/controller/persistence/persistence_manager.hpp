#pragma once
#include "config_game.hpp"
#include "leaderboard.hpp"
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
    static PersistedGame loadGame();
    static bool hasSavedGame();
    static void deleteSave();

    static bool storeLeaderboardEntry(const LeaderboardEntry entry);
    static std::vector<LeaderboardEntry> getTopNLeaderboardEntries(int topN);

    static bool saveConfig(const GameConfig &config);
    static GameConfig loadConfig();
};

} // namespace controller