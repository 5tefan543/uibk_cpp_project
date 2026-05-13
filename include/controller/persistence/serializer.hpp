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
    static constexpr auto value = object("stage", &T::stage, "wave", &T::wave, "score", &T::score, "currency",
                                         &T::currency, "playerStats", &T::playerStats);
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
               "waveDurationSeconds", &T::waveDurationSeconds, "wavesPerStage", &T::wavesPerStage, "windowConfig",
               &T::windowConfig, "assetConfig", &T::assetConfig);
};

template <>
struct glz::meta<controller::LeaderboardEntry> {
    using T = controller::LeaderboardEntry;
    static constexpr auto value = object("playerName", &T::playerName, "score", &T::score, "wave", &T::wave);
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