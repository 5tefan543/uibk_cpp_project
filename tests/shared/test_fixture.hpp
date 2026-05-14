#pragma once

#include "controller/debug/debug_context.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "shared/test_filesystem.hpp"

#include <fstream>
#include <stdexcept>

struct TestFixture {
  private:
    test::ScopedTestDirectory testDir_{"roguelike-game-test-"};

    void resetDebugContext()
    {
        controller::DebugContext &debug = controller::DebugContext::get();
        debug = controller::DebugContext();
    }

    void clearPersistenceManagerCache() { controller::PersistenceManager::resetConfig(); }

    void ensureTestGameConfig()
    {
        const std::filesystem::path targetConfig = "config/game-config.json";
        std::filesystem::create_directories(targetConfig.parent_path());

        std::ofstream out(targetConfig);
        if (!out) {
            throw std::runtime_error("Failed to create test game config file");
        }

        out << R"({
  "initialStage": 1,
  "initialWave": 1,
  "initialCurrency": 0,
  "waveDurationSeconds": 60,
  "wavesPerStage": 5,
  "windowConfig": {
    "width": 800,
    "height": 600,
    "title": "Test Game"
  },
  "assetConfig": {
    "playerTexturePath": "assets/characters/player.png",
    "enemyTexturePath": "assets/characters/enemy.png",
    "mapTexturePath": "assets/maps/test_map.png",
    "fontPath": "assets/font/BigBlueTerm_Nerd_Font/"
  }
})";
    }

  public:
    TestFixture()
    {
        resetDebugContext();
        clearPersistenceManagerCache();
        ensureTestGameConfig();
    }
    ~TestFixture() = default;
};