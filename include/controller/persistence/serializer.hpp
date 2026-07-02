#pragma once
#include "game/persisted_game.hpp"
#include "glaze_meta.hpp"
#include "leaderboard.hpp"
#include "logging/log.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <glaze/glaze.hpp>
#include <iostream>
#include <sstream>
#include <vector>

namespace controller {

class Serializer {
  public:
    Serializer() = delete;
    ~Serializer() = delete;

    static inline const std::filesystem::path configDir{"config"};
    static inline const std::filesystem::path saveFilePath = configDir / "persisted-game.json";
    static inline const std::filesystem::path leaderboardFilePath = configDir / "leaderboard.json";
    static inline const std::filesystem::path configFilePath = configDir / "game-config.json";
    static inline const std::filesystem::path storeFilePath = configDir / "persisted-store.json";

    template <typename T>
    static bool writeJsonToFile(const T &value, const std::filesystem::path &path)
    {
        std::string json;
        if (const auto err = glz::write_json(value, json)) {
            logger::log(logger::ERROR, std::format("Failed to serialize JSON for {}: {}", path.string(),
                                                   glz::format_error(err, json)));
            return false;
        }

        const auto parentPath = path.parent_path();
        if (!parentPath.empty()) {
            std::error_code ec;
            std::filesystem::create_directories(parentPath, ec);

            if (ec) {
                logger::log(logger::ERROR,
                            std::format("Failed to create directories for {}: {}", path.string(), ec.message()));
                return false;
            }
        }

        std::ofstream out(path);
        if (!out) {
            logger::log(logger::ERROR, std::format("Failed to open file for writing: {}", path.string()));
            return false;
        }

        out << json;

        if (!out) {
            logger::log(logger::ERROR, std::format("Failed to write JSON to file: {}", path.string()));
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
            logger::log(logger::ERROR, std::format("Failed to deserialize JSON for {}: {}", path.string(),
                                                   glz::format_error(err, buffer.str())));
            return false;
        }

        return true;
    }
};

} // namespace controller