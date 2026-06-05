#pragma once
#include "geometry/vector.hpp"
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

struct GameConfig {
    int initialStage;
    int initialWave;
    int initialCurrency;
    int waveDurationSeconds;
    int wavesPerStage;
    int maxEnemyCount;
    WindowConfig windowConfig;
    AssetConfig assetConfig;
    Vec2<unsigned> locTabNumBuckets = {20, 20};
    Vec2<float> mapSize = {1920.0f * 2.0f, 1080.0f * 2.0f};
};

} // namespace controller