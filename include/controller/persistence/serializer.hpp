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

// glz::meta explicit specializations must be at global (namespace) scope —
// they cannot be nested inside a class body.

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
            std::cerr << "Failed to serialize JSON for " << path << std::endl;
            return false;
        }

        std::filesystem::create_directories(path.parent_path());

        std::ofstream out(path);
        if (!out) {
            std::cerr << "Failed to open file for writing: " << path << std::endl;
            return false;
        }

        out << json;
        return true;
    }

    template <typename T>
    static bool readJsonFromFile(T &value, const std::filesystem::path &path)
    {
        std::ifstream in(saveFilePath);
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

    static std::vector<LeaderboardEntry> readLeaderboardEntriesFromDisk()
    {
        std::vector<LeaderboardEntry> entries;
        readJsonFromFile(entries, leaderboardFilePath);
        std::ranges::sort(entries, [](const LeaderboardEntry &left, const LeaderboardEntry &right) {
            return left.score > right.score;
        });
        return entries;
    }
};

} // namespace controller