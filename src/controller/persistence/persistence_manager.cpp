#include "controller/persistence/persistence_manager.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <glaze/glaze.hpp>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

namespace {

const fs::path configDir{"config"};
const fs::path saveFilePath = configDir / "saved-game.json";
const fs::path leaderboardFilePath = configDir / "leaderboard.json";
const fs::path configFilePath = configDir / "game-config.json";

struct LeaderboardEntry {
    std::string playerName;
    int score = 0;
};

template <typename T>
bool writeJsonToFile(const T &value, const fs::path &path)
{
    std::string json;
    if (const auto err = glz::write_json(value, json)) {
        std::cerr << "Failed to serialize JSON for " << path << std::endl;
        return false;
    }

    fs::create_directories(path.parent_path());

    std::ofstream out(path);
    if (!out) {
        std::cerr << "Failed to open file for writing: " << path << std::endl;
        return false;
    }

    out << json;
    return true;
}

template <typename T>
bool readJsonFromFile(T &value, const fs::path &path)
{
    std::ifstream in(path);
    if (!in) {
        return false;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();

    if (const auto err = glz::read_json(value, buffer.str())) {
        std::cerr << "Failed to deserialize JSON for " << path << std::endl;
        return false;
    }

    return true;
}

std::vector<LeaderboardEntry> readLeaderboardEntriesFromDisk()
{
    std::vector<LeaderboardEntry> entries;
    readJsonFromFile(entries, leaderboardFilePath);
    std::ranges::sort(
        entries, [](const LeaderboardEntry &left, const LeaderboardEntry &right) { return left.score > right.score; });
    return entries;
}

} // namespace

template <>
struct glz::meta<controller::PlayerStats> {
    using T = controller::PlayerStats;
    static constexpr auto value =
        object("maxHealth", &T::maxHealth, "attackPower", &T::attackPower, "attackSpeed", &T::attackSpeed, "defense",
               &T::defense, "speed", &T::speed, "hasDash", &T::hasDash);
};

template <>
struct glz::meta<controller::PersistedGame> {
    using T = controller::PersistedGame;
    static constexpr auto value =
        object("stage", &T::stage, "wave", &T::wave, "currency", &T::currency, "playerStats", &T::playerStats);
};

template <>
struct glz::meta<controller::WindowConfig> {
    using T = controller::WindowConfig;
    static constexpr auto value = object("width", &T::width, "height", &T::height, "title", &T::title);
};

template <>
struct glz::meta<controller::AssetConfig> {
    using T = controller::AssetConfig;
    static constexpr auto value =
        object("playerTexturePath", &T::playerTexturePath, "enemyTexturePath", &T::enemyTexturePath, "mapTexturePath",
               &T::mapTexturePath, "fontPath", &T::fontPath);
};

template <>
struct glz::meta<controller::GameConfig> {
    using T = controller::GameConfig;
    static constexpr auto value =
        object("initialStage", &T::initialStage, "initialWave", &T::initialWave, "initialCurrency", &T::initialCurrency,
               "windowConfig", &T::windowConfig, "assetConfig", &T::assetConfig);
};

template <>
struct glz::meta<LeaderboardEntry> {
    using T = LeaderboardEntry;
    static constexpr auto value = object("playerName", &T::playerName, "score", &T::score);
};

namespace controller {

PersistenceManager::PersistenceManager()
{
    std::cout << "PersistenceManager constructed" << std::endl;
}

PersistenceManager::~PersistenceManager()
{
    std::cout << "PersistenceManager destructed" << std::endl;
}

void PersistenceManager::saveGame(const PersistedGame &persistedGame)
{
    writeJsonToFile(persistedGame, saveFilePath);
}

void PersistenceManager::loadGame(PersistedGame &persistedGame)
{
    readJsonFromFile(persistedGame, saveFilePath);
}

bool PersistenceManager::hasSavedGame()
{
    return fs::exists(saveFilePath);
}

void PersistenceManager::deleteSave()
{
    std::error_code ec;
    fs::remove(saveFilePath, ec);
    if (ec) {
        std::cerr << "Failed to delete save file: " << saveFilePath << std::endl;
    }
}

void PersistenceManager::storeLeaderboardEntry(const std::string &playerName, int score)
{
    auto entries = readLeaderboardEntriesFromDisk();
    entries.push_back({playerName, score});
    std::ranges::sort(
        entries, [](const LeaderboardEntry &left, const LeaderboardEntry &right) { return left.score > right.score; });

    writeJsonToFile(entries, leaderboardFilePath);
}

std::vector<std::pair<std::string, int>> PersistenceManager::getLeaderboardEntries(int topN)
{
    if (topN <= 0) {
        return {};
    }

    const auto entries = readLeaderboardEntriesFromDisk();
    std::vector<std::pair<std::string, int>> result;
    result.reserve(std::min(static_cast<int>(entries.size()), topN));

    for (int i = 0; i < static_cast<int>(entries.size()) && i < topN; ++i) {
        result.emplace_back(entries[i].playerName, entries[i].score);
    }

    return result;
}

void PersistenceManager::saveConfig(const GameConfig &config)
{
    writeJsonToFile(config, configFilePath);
}

GameConfig PersistenceManager::loadConfig()
{
    GameConfig config;
    readJsonFromFile(config, configFilePath);
    return config;
}

} // namespace controller