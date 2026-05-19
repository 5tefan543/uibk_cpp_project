#pragma once

#include "controller/debug/debug_context.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "shared/test_filesystem.hpp"

#include <filesystem>
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

    void ensureGameConfigAvailable()
    {
        const std::filesystem::path sourceConfig =
            std::filesystem::path(ROGUELIKE_SOURCE_DIR) / "config" / "game-config.json";
        const std::filesystem::path targetConfig = "config/game-config.json";

        if (!std::filesystem::exists(sourceConfig)) {
            throw std::runtime_error("Missing source config file: " + sourceConfig.string());
        }

        std::filesystem::create_directories(targetConfig.parent_path());
        std::filesystem::copy_file(sourceConfig, targetConfig, std::filesystem::copy_options::overwrite_existing);
    }

  public:
    TestFixture()
    {
        resetDebugContext();
        clearPersistenceManagerCache();
        ensureGameConfigAvailable();
    }
    ~TestFixture() = default;
};