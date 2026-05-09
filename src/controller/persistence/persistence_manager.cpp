#include "controller/persistence/persistence_manager.hpp"
#include "controller/persistence/serializer.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace controller {

static std::optional<GameConfig> configCache_ = std::nullopt;

bool PersistenceManager::saveGame(const PersistedGame &persistedGame)
{
    return Serializer::writeJsonToFile(persistedGame, Serializer::saveFilePath);
}

PersistedGame PersistenceManager::loadGame()
{
    PersistedGame persistedGame;
    if (!Serializer::readJsonFromFile(persistedGame, Serializer::saveFilePath)) {
        throw std::runtime_error("Failed to load game from: " + Serializer::saveFilePath.string());
    }
    return persistedGame;
}

bool PersistenceManager::hasSavedGame()
{
    PersistedGame persistedGame;
    return Serializer::readJsonFromFile(persistedGame, Serializer::saveFilePath);
}

void PersistenceManager::deleteSave()
{
    std::error_code ec;
    fs::remove(Serializer::saveFilePath, ec);
    if (ec) {
        std::cerr << "Failed to delete save file: " << Serializer::saveFilePath << std::endl;
    }
}

bool PersistenceManager::storeLeaderboardEntry(const std::string &playerName, int score)
{
    auto entries = Serializer::readLeaderboardEntriesFromDisk();
    entries.push_back({playerName, score});
    std::ranges::sort(
        entries, [](const LeaderboardEntry &left, const LeaderboardEntry &right) { return left.score > right.score; });

    return Serializer::writeJsonToFile(entries, Serializer::leaderboardFilePath);
}

std::vector<std::pair<std::string, int>> PersistenceManager::getTopNLeaderboardEntries(int topN)
{
    if (topN <= 0) {
        return {};
    }

    const auto entries = Serializer::readLeaderboardEntriesFromDisk();
    std::vector<std::pair<std::string, int>> result;
    result.reserve(std::min(static_cast<int>(entries.size()), topN));

    for (int i = 0; i < static_cast<int>(entries.size()) && i < topN; ++i) {
        result.emplace_back(entries[i].playerName, entries[i].score);
    }

    return result;
}

bool PersistenceManager::saveConfig(const GameConfig &config)
{
    if (Serializer::writeJsonToFile(config, Serializer::configFilePath)) {
        configCache_ = config;
        return true;
    }
    return false;
}

GameConfig PersistenceManager::loadConfig()
{
    if (configCache_.has_value()) {
        return *configCache_;
    }

    GameConfig config;

    if (!Serializer::readJsonFromFile(config, Serializer::configFilePath)) {
        throw std::runtime_error("Failed to load game config from: " + Serializer::configFilePath.string());
    }

    configCache_ = config;

    return config;
}

} // namespace controller