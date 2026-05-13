#include "controller/persistence/serializer.hpp"
#include "shared/test_filesystem.hpp"
#include "shared/test_fixture.hpp"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

using namespace controller;

TEST_CASE_METHOD(TestFixture, "Serializer returns false when target file cannot be opened for writing")
{
    test::ScopedTestDirectory testDir("roguelike-serializer-test-");

    std::filesystem::create_directories(Serializer::configDir);

    PersistedGame game;
    game.stage = 1;

    // Writing to an existing directory path should fail when opening std::ofstream.
    REQUIRE_FALSE(Serializer::writeJsonToFile(game, Serializer::configDir / ""));
}

TEST_CASE_METHOD(TestFixture, "Serializer returns false when parent path exists as a file")
{
    test::ScopedTestDirectory testDir("roguelike-serializer-test-");

    {
        std::ofstream configPathAsFile(Serializer::configDir);
        REQUIRE(configPathAsFile.good());
    }

    PersistedGame game;
    game.stage = 1;

    REQUIRE_FALSE(Serializer::writeJsonToFile(game, Serializer::configDir / "persisted-game.json"));
}

TEST_CASE_METHOD(TestFixture, "Serializer returns false when stream write fails")
{
    test::ScopedTestDirectory testDir("roguelike-serializer-test-");

    if (!std::filesystem::exists("/dev/full")) {
        SKIP("/dev/full is unavailable on this platform");
    }

    GameConfig config;
    config.windowConfig.title = std::string(1 << 20, 'x');

    REQUIRE_FALSE(Serializer::writeJsonToFile(config, "/dev/full"));
}

TEST_CASE_METHOD(TestFixture, "Serializer returns false when reading from missing file")
{
    test::ScopedTestDirectory testDir("roguelike-serializer-test-");

    PersistedGame game;

    REQUIRE_FALSE(Serializer::readJsonFromFile(game, Serializer::saveFilePath));
}

TEST_CASE_METHOD(TestFixture, "Serializer returns false when deserializing invalid JSON")
{
    test::ScopedTestDirectory testDir("roguelike-serializer-test-");

    std::filesystem::create_directories(Serializer::saveFilePath.parent_path());
    std::ofstream out(Serializer::saveFilePath);
    REQUIRE(out.good());
    out << "{ invalid json";
    out.close();

    PersistedGame game;

    REQUIRE_FALSE(Serializer::readJsonFromFile(game, Serializer::saveFilePath));
}

TEST_CASE_METHOD(TestFixture, "Serializer writes and reads persisted game JSON")
{
    test::ScopedTestDirectory testDir("roguelike-serializer-test-");

    PersistedGame input;
    input.stage = 7;
    input.wave = 4;
    input.currency = 987;
    input.playerStats.maxHealth = 120.0f;
    input.playerStats.attackPower = 24.5f;
    input.playerStats.attackSpeed = 1.75f;
    input.playerStats.defense = 8.0f;
    input.playerStats.speed = 3.5f;
    input.playerStats.hasDash = true;

    REQUIRE(Serializer::writeJsonToFile(input, Serializer::saveFilePath));
    REQUIRE(std::filesystem::exists(Serializer::saveFilePath));

    PersistedGame output{};
    REQUIRE(Serializer::readJsonFromFile(output, Serializer::saveFilePath));

    REQUIRE(output.stage == 7);
    REQUIRE(output.wave == 4);
    REQUIRE(output.currency == 987);
    REQUIRE(output.playerStats.maxHealth == Catch::Approx(120.0f));
    REQUIRE(output.playerStats.attackPower == Catch::Approx(24.5f));
    REQUIRE(output.playerStats.attackSpeed == Catch::Approx(1.75f));
    REQUIRE(output.playerStats.defense == Catch::Approx(8.0f));
    REQUIRE(output.playerStats.speed == Catch::Approx(3.5f));
    REQUIRE(output.playerStats.hasDash == true);
}

TEST_CASE_METHOD(TestFixture, "Serializer creates missing parent directories while writing")
{
    test::ScopedTestDirectory testDir("roguelike-serializer-test-");

    const auto nestedPath = Serializer::configDir / "nested" / "persisted-game.json";
    REQUIRE_FALSE(std::filesystem::exists(nestedPath.parent_path()));

    PersistedGame game{};
    game.stage = 2;

    REQUIRE(Serializer::writeJsonToFile(game, nestedPath));
    REQUIRE(std::filesystem::exists(nestedPath.parent_path()));
    REQUIRE(std::filesystem::exists(nestedPath));
}

TEST_CASE_METHOD(TestFixture, "Serializer overwrites existing JSON file")
{
    test::ScopedTestDirectory testDir("roguelike-serializer-test-");

    PersistedGame first{};
    first.stage = 1;
    first.wave = 1;
    first.currency = 10;

    PersistedGame second{};
    second.stage = 9;
    second.wave = 8;
    second.currency = 700;

    REQUIRE(Serializer::writeJsonToFile(first, Serializer::saveFilePath));
    REQUIRE(Serializer::writeJsonToFile(second, Serializer::saveFilePath));

    PersistedGame output{};
    REQUIRE(Serializer::readJsonFromFile(output, Serializer::saveFilePath));
    REQUIRE(output.stage == 9);
    REQUIRE(output.wave == 8);
    REQUIRE(output.currency == 700);
}

TEST_CASE_METHOD(TestFixture, "Serializer writes and reads leaderboard entry vectors")
{
    test::ScopedTestDirectory testDir("roguelike-serializer-test-");

    const std::vector<LeaderboardEntry> input{{"Alice", 1200, 12}, {"Bob", 900, 10}};

    REQUIRE(Serializer::writeJsonToFile(input, Serializer::leaderboardFilePath));

    std::vector<LeaderboardEntry> output;
    REQUIRE(Serializer::readJsonFromFile(output, Serializer::leaderboardFilePath));
    REQUIRE(output.size() == 2);
    REQUIRE(output[0] == LeaderboardEntry{"Alice", 1200, 12});
    REQUIRE(output[1] == LeaderboardEntry{"Bob", 900, 10});
}
