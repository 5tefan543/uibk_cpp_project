#include "controller/persistence/serializer.hpp"
#include "shared/test_filesystem.hpp"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

using namespace controller;

TEST_CASE("Serializer returns false when target file cannot be opened for writing")
{
    test::ScopedTestDirectory testDir("roguelike-serializer-test-");

    std::filesystem::create_directories(Serializer::configDir);

    PersistedGame game;
    game.stage = 1;

    // Writing to an existing directory path should fail when opening std::ofstream.
    REQUIRE_FALSE(Serializer::writeJsonToFile(game, Serializer::configDir / ""));
}

TEST_CASE("Serializer returns false when reading from missing file")
{
    test::ScopedTestDirectory testDir("roguelike-serializer-test-");

    PersistedGame game;

    REQUIRE_FALSE(Serializer::readJsonFromFile(game, Serializer::saveFilePath));
}

TEST_CASE("Serializer returns false when deserializing invalid JSON")
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
