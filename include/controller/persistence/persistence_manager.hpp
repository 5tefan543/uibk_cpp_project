#pragma once
#include "config/game_config.hpp"
#include "game/persisted_game.hpp"
#include "game/store/persisted_store.hpp"
#include "leaderboard.hpp"
#include <optional>
#include <string>
#include <vector>
namespace controller {

class PersistenceManager {
  public:
    PersistenceManager() = delete;
    ~PersistenceManager() = delete;

    static bool saveGame(const game::PersistedGame &persistedGame);
    static game::PersistedGame loadGame();
    static bool hasSavedGame();
    static void deleteSave();

    static bool storeLeaderboardEntry(const LeaderboardEntry entry);
    static std::vector<LeaderboardEntry> getLeaderboardEntries();
    static std::vector<LeaderboardEntry> getTopNLeaderboardEntries(int topN);

    static bool saveConfig(const config::GameConfig &config);
    static const config::GameConfig &getConfig();
    static void resetConfig();

    static bool saveStore(const game::PersistedStore &persistedStore);
    static std::optional<game::PersistedStore> getStore();
    static void deleteStore();
};

} // namespace controller