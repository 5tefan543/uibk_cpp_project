#include "controller/persistence/persistence_manager.hpp"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <filesystem>

using namespace controller;

namespace {

namespace fs = std::filesystem;

class ScopedCurrentPath {
  public:
    explicit ScopedCurrentPath(const fs::path &newPath) : oldPath_(fs::current_path())
    {
        fs::create_directories(newPath);
        fs::current_path(newPath);
    }

    ~ScopedCurrentPath() { fs::current_path(oldPath_); }

  private:
    fs::path oldPath_;
};

fs::path createTempTestDirectory()
{
    const auto uniquePart = std::chrono::steady_clock::now().time_since_epoch().count();
    const fs::path dir = fs::temp_directory_path() / ("roguelike-persistence-test-" + std::to_string(uniquePart));
    fs::create_directories(dir);
    return dir;
}

} // namespace

TEST_CASE("PersistenceManager saves and loads game state")
{
    const auto testDir = createTempTestDirectory();
    {
        ScopedCurrentPath pathGuard(testDir);

        PersistedGame input;
        input.stage = 4;
        input.wave = 9;
        input.currency = 321;
        input.playerStats.attackPower = 42.0f;
        input.playerStats.hasDash = true;

        {
            PersistenceManager manager;
            manager.saveGame(input);
        }

        PersistedGame output;
        output.stage = 0;
        output.wave = 0;
        output.currency = -1;
        output.playerStats.attackPower = -1.0f;
        output.playerStats.hasDash = false;

        {
            PersistenceManager manager;
            manager.loadGame(output);
        }

        REQUIRE(output.stage == 4);
        REQUIRE(output.wave == 9);
        REQUIRE(output.currency == 321);
        REQUIRE(output.playerStats.attackPower == Catch::Approx(42.0f));
        REQUIRE(output.playerStats.hasDash == true);
    }

    fs::remove_all(testDir);
}

TEST_CASE("PersistenceManager reports and deletes save file")
{
    const auto testDir = createTempTestDirectory();
    {
        ScopedCurrentPath pathGuard(testDir);

        PersistenceManager manager;
        PersistedGame game;
        game.stage = 2;
        manager.saveGame(game);

        REQUIRE(manager.hasSavedGame() == true);

        manager.deleteSave();

        REQUIRE(manager.hasSavedGame() == false);
    }

    fs::remove_all(testDir);
}

TEST_CASE("PersistenceManager stores leaderboard entries sorted by score")
{
    const auto testDir = createTempTestDirectory();
    {
        ScopedCurrentPath pathGuard(testDir);

        PersistenceManager manager;
        manager.storeLeaderboardEntry("Alice", 120);
        manager.storeLeaderboardEntry("Bob", 450);
        manager.storeLeaderboardEntry("Carol", 300);

        const auto topTwo = manager.getLeaderboardEntries(2);

        REQUIRE(topTwo.size() == 2);
        REQUIRE(topTwo[0].first == "Bob");
        REQUIRE(topTwo[0].second == 450);
        REQUIRE(topTwo[1].first == "Carol");
        REQUIRE(topTwo[1].second == 300);
    }

    fs::remove_all(testDir);
}

TEST_CASE("PersistenceManager saves and loads config")
{
    const auto testDir = createTempTestDirectory();
    {
        ScopedCurrentPath pathGuard(testDir);

        GameConfig input;
        input.initialStage = 7;
        input.initialWave = 3;
        input.initialCurrency = 999;
        input.windowConfig.width = 1280;
        input.windowConfig.height = 720;
        input.windowConfig.title = "Test Window";
        input.assetConfig.playerTexturePath = "assets/players/test_player.png";
        input.assetConfig.enemyTexturePath = "assets/enemies/test_enemy.png";
        input.assetConfig.mapTexturePath = "assets/maps/test_map.png";
        input.assetConfig.fontPath = "assets/fonts/test_font.ttf";

        {
            PersistenceManager manager;
            manager.saveConfig(input);
        }

        GameConfig output;
        {
            PersistenceManager manager;
            output = manager.loadConfig();
        }

        REQUIRE(output.initialStage == 7);
        REQUIRE(output.initialWave == 3);
        REQUIRE(output.initialCurrency == 999);
        REQUIRE(output.windowConfig.width == 1280);
        REQUIRE(output.windowConfig.height == 720);
        REQUIRE(output.windowConfig.title == "Test Window");
        REQUIRE(output.assetConfig.playerTexturePath == "assets/players/test_player.png");
        REQUIRE(output.assetConfig.enemyTexturePath == "assets/enemies/test_enemy.png");
        REQUIRE(output.assetConfig.mapTexturePath == "assets/maps/test_map.png");
        REQUIRE(output.assetConfig.fontPath == "assets/fonts/test_font.ttf");
    }

    fs::remove_all(testDir);
}

TEST_CASE("PersistenceManager returns empty leaderboard for non-positive topN")
{
    const auto testDir = createTempTestDirectory();
    {
        ScopedCurrentPath pathGuard(testDir);

        PersistenceManager manager;
        manager.storeLeaderboardEntry("Alice", 100);

        REQUIRE(manager.getLeaderboardEntries(0).empty());
        REQUIRE(manager.getLeaderboardEntries(-4).empty());
    }

    fs::remove_all(testDir);
}
