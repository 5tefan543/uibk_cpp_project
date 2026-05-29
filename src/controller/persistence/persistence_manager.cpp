#include "controller/persistence/persistence_manager.hpp"
#include "controller/persistence/serializer.hpp"
#include <algorithm>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace controller {

static std::optional<GameConfig> configCache = std::nullopt;

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
        logger::log(logger::DEBUG, std::format("Failed to delete save file: {}", Serializer::saveFilePath.c_str()));
    }
}

std::vector<LeaderboardEntry> PersistenceManager::getLeaderboardEntries()
{
    std::vector<LeaderboardEntry> entries;
    Serializer::readJsonFromFile(entries, Serializer::leaderboardFilePath);
    std::ranges::sort(
        entries, [](const LeaderboardEntry &left, const LeaderboardEntry &right) { return left.score > right.score; });
    return entries;
}

bool PersistenceManager::storeLeaderboardEntry(const LeaderboardEntry entry)
{
    auto entries = getLeaderboardEntries();
    entries.push_back(entry);
    std::ranges::sort(
        entries, [](const LeaderboardEntry &left, const LeaderboardEntry &right) { return left.score > right.score; });

    return Serializer::writeJsonToFile(entries, Serializer::leaderboardFilePath);
}

std::vector<LeaderboardEntry> PersistenceManager::getTopNLeaderboardEntries(int topN)
{
    if (topN <= 0) {
        return {};
    }

    const auto entries = getLeaderboardEntries();
    const auto count = std::min(static_cast<std::size_t>(topN), entries.size());
    return {entries.begin(), entries.begin() + count};
}

bool PersistenceManager::saveConfig(const GameConfig &config)
{
    if (Serializer::writeJsonToFile(config, Serializer::configFilePath)) {
        configCache = config;
        return true;
    }
    return false;
}

GameConfig PersistenceManager::getConfig()
{
    if (configCache.has_value()) {
        return *configCache;
    }

    GameConfig config;

    if (!Serializer::readJsonFromFile(config, Serializer::configFilePath)) {
        throw std::runtime_error("Failed to load game config from: " + Serializer::configFilePath.string());
    }

    configCache = config;

    return config;
}

void PersistenceManager::resetConfig()
{
    configCache.reset();
}

} // namespace controller