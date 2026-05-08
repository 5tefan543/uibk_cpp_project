#include "controller/persistence/persistence_manager.hpp"
#include "shared/test_filesystem.hpp"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace controller;

TEST_CASE("PersistenceManager saves and loads game state")
{
    test::ScopedTestDirectory testDir("roguelike-persistence-test-");

    PersistedGame input;
    input.stage = 4;
    input.wave = 9;
    input.currency = 321;
    input.playerStats.attackPower = 42.0f;
    input.playerStats.hasDash = true;

    PersistenceManager::saveGame(input);

    PersistedGame output;
    output.stage = 0;
    output.wave = 0;
    output.currency = -1;
    output.playerStats.attackPower = -1.0f;
    output.playerStats.hasDash = false;

    PersistenceManager::loadGame(output);

    REQUIRE(output.stage == 4);
    REQUIRE(output.wave == 9);
    REQUIRE(output.currency == 321);
    REQUIRE(output.playerStats.attackPower == Catch::Approx(42.0f));
    REQUIRE(output.playerStats.hasDash == true);
}

TEST_CASE("PersistenceManager reports and deletes save file")
{
    test::ScopedTestDirectory testDir("roguelike-persistence-test-");

    PersistedGame game;
    game.stage = 2;
    PersistenceManager::saveGame(game);

    REQUIRE(PersistenceManager::hasSavedGame() == true);

    PersistenceManager::deleteSave();

    REQUIRE(PersistenceManager::hasSavedGame() == false);
}

TEST_CASE("PersistenceManager stores leaderboard entries sorted by score")
{
    test::ScopedTestDirectory testDir("roguelike-persistence-test-");

    PersistenceManager::storeLeaderboardEntry("Alice", 120);
    PersistenceManager::storeLeaderboardEntry("Bob", 450);
    PersistenceManager::storeLeaderboardEntry("Carol", 300);

    const auto topTwo = PersistenceManager::getTopNLeaderboardEntries(2);

    REQUIRE(topTwo.size() == 2);
    REQUIRE(topTwo[0].first == "Bob");
    REQUIRE(topTwo[0].second == 450);
    REQUIRE(topTwo[1].first == "Carol");
    REQUIRE(topTwo[1].second == 300);
}

TEST_CASE("PersistenceManager saves and loads config")
{
    test::ScopedTestDirectory testDir("roguelike-persistence-test-");

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

    PersistenceManager::saveConfig(input);

    GameConfig output;
    output = PersistenceManager::loadConfig();

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

TEST_CASE("PersistenceManager returns empty leaderboard for non-positive topN")
{
    test::ScopedTestDirectory testDir("roguelike-persistence-test-");

    PersistenceManager::storeLeaderboardEntry("Alice", 100);

    REQUIRE(PersistenceManager::getTopNLeaderboardEntries(0).empty());
    REQUIRE(PersistenceManager::getTopNLeaderboardEntries(-4).empty());
}
