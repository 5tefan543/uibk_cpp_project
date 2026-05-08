#include "controller/persistence/persistence_manager.hpp"
#include "controller/persistence/serializer.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace controller {

std::optional<GameConfig> PersistenceManager::configCache_ = std::nullopt;

void PersistenceManager::saveGame(const PersistedGame &persistedGame)
{
    Serializer::writeJsonToFile(persistedGame, Serializer::saveFilePath);
}

void PersistenceManager::loadGame(PersistedGame &persistedGame)
{
    Serializer::readJsonFromFile(persistedGame, Serializer::saveFilePath);
}

bool PersistenceManager::hasSavedGame()
{
    return fs::exists(Serializer::saveFilePath);
}

void PersistenceManager::deleteSave()
{
    std::error_code ec;
    fs::remove(Serializer::saveFilePath, ec);
    if (ec) {
        std::cerr << "Failed to delete save file: " << Serializer::saveFilePath << std::endl;
    }
}

void PersistenceManager::storeLeaderboardEntry(const std::string &playerName, int score)
{
    auto entries = Serializer::readLeaderboardEntriesFromDisk();
    entries.push_back({playerName, score});
    std::ranges::sort(
        entries, [](const LeaderboardEntry &left, const LeaderboardEntry &right) { return left.score > right.score; });

    Serializer::writeJsonToFile(entries, Serializer::leaderboardFilePath);
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

void PersistenceManager::saveConfig(const GameConfig &config)
{
    Serializer::writeJsonToFile(config, Serializer::configFilePath);
    configCache_ = config;
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