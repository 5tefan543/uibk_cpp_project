#include "controller/persistence/persistence_manager.hpp"
#include "controller/persistence/serializer.hpp"
#include <algorithm>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace controller {

static std::optional<config::GameConfig> configCache = std::nullopt;

bool PersistenceManager::saveGame(const game::PersistedGame &persistedGame)
{
    return Serializer::writeJsonToFile(persistedGame, Serializer::saveFilePath);
}

game::PersistedGame PersistenceManager::loadGame()
{
    game::PersistedGame persistedGame;
    if (!Serializer::readJsonFromFile(persistedGame, Serializer::saveFilePath)) {
        throw std::runtime_error("Failed to load game from: " + Serializer::saveFilePath.string());
    }
    return persistedGame;
}

bool PersistenceManager::hasSavedGame()
{
    game::PersistedGame persistedGame;
    return Serializer::readJsonFromFile(persistedGame, Serializer::saveFilePath);
}

void PersistenceManager::deleteSave()
{
    std::error_code ec;
    fs::remove(Serializer::saveFilePath, ec);
    if (ec) {
        logger::log(logger::DEBUG, std::format("Failed to delete save file: {}", Serializer::saveFilePath.string()));
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

bool PersistenceManager::saveConfig(const config::GameConfig &config)
{
    if (Serializer::writeJsonToFile(config, Serializer::configFilePath)) {
        configCache = config;
        return true;
    }
    return false;
}

const config::GameConfig &PersistenceManager::getConfig()
{
    if (configCache.has_value()) {
        return *configCache;
    }

    config::GameConfig config;

    if (!Serializer::readJsonFromFile(config, Serializer::configFilePath)) {
        throw std::runtime_error("Failed to load game config from: " + Serializer::configFilePath.string());
    }

    configCache = std::move(config);

    return *configCache;
}

void PersistenceManager::resetConfig()
{
    configCache.reset();
}

static std::optional<game::PersistedStore> storeCache = std::nullopt;

bool PersistenceManager::saveStore(const game::PersistedStore &persistedStore)
{
    return Serializer::writeJsonToFile(persistedStore, Serializer::storeFilePath);
}

std::optional<game::PersistedStore> PersistenceManager::getStore()
{
    if (storeCache.has_value()) {
        return storeCache;
    }

    game::PersistedStore persistedStore;

    if (!Serializer::readJsonFromFile(persistedStore, Serializer::storeFilePath)) {
        return std::nullopt;
    }

    storeCache = std::move(persistedStore);

    return storeCache;
}

void PersistenceManager::deleteStore()
{
    std::error_code ec;
    fs::remove(Serializer::storeFilePath, ec);
    if (ec) {
        logger::log(logger::DEBUG, std::format("Failed to delete store file: {}", Serializer::storeFilePath.string()));
        return;
    }
    storeCache.reset();
}

} // namespace controller