#pragma once
#include "geometry/vector.hpp"
#include "logging/log.hpp"
#include <string>

namespace controller {

struct WindowConfig {
    int width;
    int height;
    std::string title;
};

struct AssetConfig {
    std::string playerTexturePath;
    std::string enemyTexturePath;
    std::string mapTexturePath;
    std::string fontPath;
};

struct LogConfig {
    logger::LogLevel level;
    bool useColor;
};

struct GameConfig {
    int initialStage;
    int initialWave;
    int initialCurrency;
    int waveDurationSeconds;
    int wavesPerStage;
    int maxEnemyCount;
    WindowConfig windowConfig;
    LogConfig logConfig;
    AssetConfig assetConfig;
    Vec2<unsigned> locTabNumBuckets;
    Vec2<float> mapSize;
};

} // namespace controller