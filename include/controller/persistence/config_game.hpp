#pragma once
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
    WindowConfig windowConfig;
    AssetConfig assetConfig;
};

} // namespace controller