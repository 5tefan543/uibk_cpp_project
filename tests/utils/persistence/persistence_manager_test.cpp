#include "controller/persistence/persistence_manager.hpp"
#include "controller/persistence/serializer.hpp"
#include "shared/test_filesystem.hpp"
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
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

    auto output = PersistenceManager::loadGame();

    REQUIRE(output.stage == 4);
    REQUIRE(output.wave == 9);
    REQUIRE(output.currency == 321);
    REQUIRE(output.playerStats.attackPower == Catch::Approx(42.0f));
    REQUIRE(output.playerStats.hasDash == true);
}

TEST_CASE("PersistenceManager throws when loading game without save")
{
    test::ScopedTestDirectory testDir("roguelike-persistence-test-");

    REQUIRE_FALSE(PersistenceManager::hasSavedGame());
    REQUIRE_THROWS_WITH(PersistenceManager::loadGame(), Catch::Matchers::ContainsSubstring("config/persisted-game.json"));
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

TEST_CASE("PersistenceManager deleteSave is safe when no save exists")
{
    test::ScopedTestDirectory testDir("roguelike-persistence-test-");

    REQUIRE_NOTHROW(PersistenceManager::deleteSave());
    REQUIRE_FALSE(PersistenceManager::hasSavedGame());
}

TEST_CASE("PersistenceManager stores leaderboard entries sorted by score")
{
    test::ScopedTestDirectory testDir("roguelike-persistence-test-");

    PersistenceManager::storeLeaderboardEntry({"Alice", 120, 1});
    PersistenceManager::storeLeaderboardEntry({"Bob", 450, 2});
    PersistenceManager::storeLeaderboardEntry({"Carol", 300, 3});

    const auto topTwo = PersistenceManager::getTopNLeaderboardEntries(2);

    REQUIRE(topTwo.size() == 2);
    REQUIRE(topTwo[0].playerName == "Bob");
    REQUIRE(topTwo[0].score == 450);
    REQUIRE(topTwo[1].playerName == "Carol");
    REQUIRE(topTwo[1].score == 300);
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

TEST_CASE("PersistenceManager loadConfig prefers cached config after save")
{
    test::ScopedTestDirectory testDir("roguelike-persistence-test-");

    GameConfig cachedConfig;
    cachedConfig.initialStage = 11;
    cachedConfig.initialWave = 6;
    cachedConfig.initialCurrency = 555;

    REQUIRE(PersistenceManager::saveConfig(cachedConfig));

    GameConfig diskConfig = cachedConfig;
    diskConfig.initialStage = 99;
    REQUIRE(Serializer::writeJsonToFile(diskConfig, Serializer::configFilePath));

    const auto output = PersistenceManager::loadConfig();

    REQUIRE(output.initialStage == 11);
    REQUIRE(output.initialWave == 6);
    REQUIRE(output.initialCurrency == 555);
}

TEST_CASE("PersistenceManager returns empty leaderboard for non-positive topN")
{
    test::ScopedTestDirectory testDir("roguelike-persistence-test-");

    PersistenceManager::storeLeaderboardEntry({"Alice", 100, 1});

    REQUIRE(PersistenceManager::getTopNLeaderboardEntries(0).empty());
    REQUIRE(PersistenceManager::getTopNLeaderboardEntries(-4).empty());
}
