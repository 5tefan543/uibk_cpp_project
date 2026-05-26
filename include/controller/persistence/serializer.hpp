#pragma once
#include "config_game.hpp"
#include "leaderboard.hpp"
#include "persisted_game.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <glaze/glaze.hpp>
#include <iostream>
#include <sstream>
#include <vector>

template <>
struct glz::meta<game::DamageKind> {
    static constexpr auto value =
        glz::enumerate("Projectile", game::DamageKind::Projectile, "MeleeArc", game::DamageKind::MeleeArc, "Beam",
                       game::DamageKind::Beam, "Area", game::DamageKind::Area);
};

template <>
struct glz::meta<game::Position> {
    static constexpr auto value = glz::object("x", &game::Position::x, "y", &game::Position::y);
};

template <>
struct glz::meta<game::PlayerStats> {
    static constexpr auto value =
        glz::object("maxHealth", &game::PlayerStats::maxHealth, "health", &game::PlayerStats::health, "attackPower",
                    &game::PlayerStats::attackPower, "attackSpeed", &game::PlayerStats::attackSpeed, "defense",
                    &game::PlayerStats::defense, "moveSpeed", &game::PlayerStats::moveSpeed, "speedOfAttack",
                    &game::PlayerStats::speedOfAttack, "attackRange", &game::PlayerStats::attackRange, "hasDash",
                    &game::PlayerStats::hasDash, "dmgKind", &game::PlayerStats::dmgKind, "enemiesPierced",
                    &game::PlayerStats::enemiesPierced, "score", &game::PlayerStats::score, "currency",
                    &game::PlayerStats::currency);
};

template <>
struct glz::meta<controller::PersistedGame> {
    static constexpr auto value =
        glz::object("wave", &controller::PersistedGame::wave, "position", &controller::PersistedGame::position,
                    "playerStats", &controller::PersistedGame::playerStats);
};

template <>
struct glz::meta<controller::WindowConfig> {
    static constexpr auto value =
        glz::object("width", &controller::WindowConfig::width, "height", &controller::WindowConfig::height, "title",
                    &controller::WindowConfig::title);
};

template <>
struct glz::meta<controller::AssetConfig> {
    static constexpr auto value = glz::object(
        "playerTexturePath", &controller::AssetConfig::playerTexturePath, "enemyTexturePath",
        &controller::AssetConfig::enemyTexturePath, "mapTexturePath", &controller::AssetConfig::mapTexturePath,
        "fontPath", &controller::AssetConfig::fontPath, "projectilePath", &controller::AssetConfig::projectilePath,
        "meleePath", &controller::AssetConfig::meleePath);
};

template <>
struct glz::meta<controller::GameConfig> {
    static constexpr auto value = glz::object(
        "initialStage", &controller::GameConfig::initialStage, "initialWave", &controller::GameConfig::initialWave,
        "initialCurrency", &controller::GameConfig::initialCurrency, "waveDurationSeconds",
        &controller::GameConfig::waveDurationSeconds, "wavesPerStage", &controller::GameConfig::wavesPerStage,
        "maxEnemyCount", &controller::GameConfig::maxEnemyCount, "windowConfig", &controller::GameConfig::windowConfig,
        "assetConfig", &controller::GameConfig::assetConfig);
};

template <>
struct glz::meta<controller::LeaderboardEntry> {
    static constexpr auto value =
        glz::object("playerName", &controller::LeaderboardEntry::playerName, "score",
                    &controller::LeaderboardEntry::score, "wave", &controller::LeaderboardEntry::wave);
};

namespace controller {

class Serializer {
  public:
    Serializer() = delete;
    ~Serializer() = delete;

    static inline const std::filesystem::path configDir{"config"};
    static inline const std::filesystem::path saveFilePath = configDir / "persisted-game.json";
    static inline const std::filesystem::path leaderboardFilePath = configDir / "leaderboard.json";
    static inline const std::filesystem::path configFilePath = configDir / "game-config.json";

    template <typename T>
    static bool writeJsonToFile(const T &value, const std::filesystem::path &path)
    {
        std::string json;
        if (const auto err = glz::write_json(value, json)) {
            std::cerr << "Failed to serialize JSON for " << path << ": " << glz::format_error(err, json) << '\n';
            return false;
        }

        const auto parentPath = path.parent_path();
        if (!parentPath.empty()) {
            std::error_code ec;
            std::filesystem::create_directories(parentPath, ec);

            if (ec) {
                std::cerr << "Failed to create directories for " << path << ": " << ec.message() << '\n';
                return false;
            }
        }

        std::ofstream out(path);
        if (!out) {
            std::cerr << "Failed to open file for writing: " << path << std::endl;
            return false;
        }

        out << json;

        if (!out) {
            std::cerr << "Failed to write JSON to file: " << path << '\n';
            return false;
        }
        return true;
    }

    template <typename T>
    static bool readJsonFromFile(T &value, const std::filesystem::path &path)
    {
        std::ifstream in(path);
        if (!in) {
            return false;
        }

        std::stringstream buffer;
        buffer << in.rdbuf();

        if (const auto err = glz::read_json(value, buffer.str())) {
            std::cerr << "Failed to deserialize JSON for " << path << ": " << glz::format_error(err, buffer.str())
                      << '\n';
            return false;
        }

        return true;
    }
};

} // namespace controller