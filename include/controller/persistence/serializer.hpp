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
#ifdef LOG_STDOUT
            std::cerr << "Failed to serialize JSON for " << path << ": " << glz::format_error(err, json) << '\n';
#endif
            return false;
        }

        const auto parentPath = path.parent_path();
        if (!parentPath.empty()) {
            std::error_code ec;
            std::filesystem::create_directories(parentPath, ec);

            if (ec) {
#ifdef LOG_STDOUT
                std::cerr << "Failed to create directories for " << path << ": " << ec.message() << '\n';
#endif
                return false;
            }
        }

        std::ofstream out(path);
        if (!out) {
#ifdef LOG_STDOUT
            std::cerr << "Failed to open file for writing: " << path << std::endl;
#endif
            return false;
        }

        out << json;

        if (!out) {
#ifdef LOG_STDOUT
            std::cerr << "Failed to write JSON to file: " << path << '\n';
#endif
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
#ifdef LOG_STDOUT
            std::cerr << "Failed to deserialize JSON for " << path << ": " << glz::format_error(err, buffer.str())
                      << '\n';
#endif
            return false;
        }

        return true;
    }
};

} // namespace controller