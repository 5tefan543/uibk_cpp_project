#pragma once
#include <string>

namespace controller {

struct WindowConfig {
    int width = 1920;
    int height = 1080;
    std::string title = "My Game";
};

struct AssetConfig {
    std::string playerTexturePath = "assets/characters/player_1_";
    std::string enemyTexturePath = "assets/characters/enemy_1_";
    std::string mapTexturePath = "assets/map/map_1.png";
    std::string fontPath = "assets/font/BigBlueTerm_Nerd_Font/BigBlueTerm437NerdFont-Regular.ttf";
};

struct GameConfig {
    int initialStage = 1;
    int initialWave = 1;
    int initialCurrency = 0;
    WindowConfig windowConfig;
    AssetConfig assetConfig;
};

} // namespace controller