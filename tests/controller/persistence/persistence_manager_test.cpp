#include "controller/persistence/persistence_manager.hpp"
#include "controller/persistence/serializer.hpp"
#include "shared/test_fixture.hpp"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <filesystem>
#include <fstream>
#include <string>

using namespace controller;

void createSavedGameFile()
{
    game::PersistedGame game;
    game.wave = 2;
    game.playerStats.currency = 150;
    game.playerStats.moveSpeed = 444.0f;
    game.playerStats.hasDash = false;
    game.playerStats.attackPower = 55.0f;
    game.playerStats.attackSpeed = 1.5f;
    game.playerStats.defense = 20.0f;
    PersistenceManager::saveGame(game);
}

TEST_CASE_METHOD(TestFixture, "PersistenceManager saves and loads game state")
{
    game::PersistedGame input;
    input.wave = 9;
    input.playerStats.currency = 321;
    input.playerStats.attackPower = 42.0f;
    input.playerStats.hasDash = true;

    PersistenceManager::saveGame(input);

    auto output = PersistenceManager::loadGame();

    REQUIRE(output.wave == 9);
    REQUIRE(output.playerStats.currency == 321);
    REQUIRE(output.playerStats.attackPower == Catch::Approx(42.0f));
    REQUIRE(output.playerStats.hasDash == true);
}

TEST_CASE_METHOD(TestFixture, "PersistenceManager throws when loading game without save")
{
    PersistenceManager::deleteSave();
    REQUIRE_FALSE(PersistenceManager::hasSavedGame());

    try {
        (void)PersistenceManager::loadGame();
        FAIL("Expected PersistenceManager::loadGame() to throw std::runtime_error");
    } catch (const std::runtime_error &error) {
        const std::string message = error.what();
        REQUIRE(message.find("Failed to load game from:") != std::string::npos);
        REQUIRE(message.find("persisted-game.json") != std::string::npos);
    }
}

TEST_CASE_METHOD(TestFixture, "PersistenceManager reports and deletes save file")
{
    game::PersistedGame game;
    game.wave = 2;
    PersistenceManager::saveGame(game);

    REQUIRE(PersistenceManager::hasSavedGame() == true);

    PersistenceManager::deleteSave();

    REQUIRE(PersistenceManager::hasSavedGame() == false);
}

TEST_CASE_METHOD(TestFixture, "PersistenceManager deleteSave is safe when no save exists")
{
    REQUIRE_NOTHROW(PersistenceManager::deleteSave());
    REQUIRE_FALSE(PersistenceManager::hasSavedGame());
}

TEST_CASE_METHOD(TestFixture, "PersistenceManager stores leaderboard entries sorted by score")
{
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

TEST_CASE_METHOD(TestFixture, "PersistenceManager loadConfig reads from disk when cache is empty")
{
    config::GameConfig input;
    input.initialStage = 13;
    input.initialWave = 8;
    input.initialCurrency = 456;
    input.windowConfig.width = 1024;
    input.windowConfig.height = 768;
    input.windowConfig.title = "Disk Config";
    input.logConfig.level = logger::WARNING;
    input.logConfig.useColor = false;

    REQUIRE(Serializer::writeJsonToFile(input, Serializer::configFilePath));

    const config::GameConfig &output = PersistenceManager::getConfig();

    REQUIRE(output.initialStage == 13);
    REQUIRE(output.initialWave == 8);
    REQUIRE(output.initialCurrency == 456);
    REQUIRE(output.windowConfig.width == 1024);
    REQUIRE(output.windowConfig.height == 768);
    REQUIRE(output.windowConfig.title == "Disk Config");
    REQUIRE(output.logConfig.level == logger::WARNING);
    REQUIRE(output.logConfig.useColor == false);
}

TEST_CASE_METHOD(TestFixture, "PersistenceManager loadConfig throws when config is missing and cache is empty")
{
    std::filesystem::remove(Serializer::configFilePath);

    try {
        (void)PersistenceManager::getConfig();
        FAIL("Expected PersistenceManager::loadConfig() to throw std::runtime_error");
    } catch (const std::runtime_error &error) {
        const std::string message = error.what();
        REQUIRE(message.find("Failed to load game config from:") != std::string::npos);
        REQUIRE(message.find("game-config.json") != std::string::npos);
    }
}

TEST_CASE_METHOD(TestFixture, "PersistenceManager saveConfig returns false when config path parent cannot be created")
{
    std::filesystem::remove_all(Serializer::configDir);

    {
        std::ofstream configPathAsFile(Serializer::configDir);
        REQUIRE(configPathAsFile.good());
    }

    config::GameConfig config;
    config.initialStage = 1;

    REQUIRE_FALSE(PersistenceManager::saveConfig(config));
}

TEST_CASE_METHOD(TestFixture, "PersistenceManager saves and loads config")
{
    config::GameConfig input;
    input.initialStage = 7;
    input.initialWave = 3;
    input.initialCurrency = 999;
    input.windowConfig.width = 1280;
    input.windowConfig.height = 720;
    input.windowConfig.title = "Test Window";

    PersistenceManager::saveConfig(input);

    const config::GameConfig &output = PersistenceManager::getConfig();

    REQUIRE(output.initialStage == 7);
    REQUIRE(output.initialWave == 3);
    REQUIRE(output.initialCurrency == 999);
    REQUIRE(output.windowConfig.width == 1280);
    REQUIRE(output.windowConfig.height == 720);
    REQUIRE(output.windowConfig.title == "Test Window");
}

TEST_CASE_METHOD(TestFixture, "PersistenceManager loadConfig prefers cached config after save")
{
    config::GameConfig cachedConfig;
    cachedConfig.initialStage = 11;
    cachedConfig.initialWave = 6;
    cachedConfig.initialCurrency = 555;

    REQUIRE(PersistenceManager::saveConfig(cachedConfig));

    config::GameConfig diskConfig = cachedConfig;
    diskConfig.initialStage = 99;
    REQUIRE(Serializer::writeJsonToFile(diskConfig, Serializer::configFilePath));

    const config::GameConfig &output = PersistenceManager::getConfig();

    REQUIRE(output.initialStage == 11);
    REQUIRE(output.initialWave == 6);
    REQUIRE(output.initialCurrency == 555);
}

TEST_CASE_METHOD(TestFixture, "PersistenceManager returns empty leaderboard for non-positive topN")
{
    PersistenceManager::storeLeaderboardEntry({"Alice", 100, 1});

    REQUIRE(PersistenceManager::getTopNLeaderboardEntries(0).empty());
    REQUIRE(PersistenceManager::getTopNLeaderboardEntries(-4).empty());
}
