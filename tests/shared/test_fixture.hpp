#pragma once

#include "controller/debug/debug_context.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "controller/persistence/serializer.hpp"
#include "shared/test_filesystem.hpp"

#include "util.hpp"
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

    void ensureGameConfigAvailable()
    {

        const std::filesystem::path sourceConfig =
            std::filesystem::path(testDir_.oldPath()) / controller::Serializer::configFilePath;
        const std::filesystem::path targetConfig = controller::Serializer::configFilePath;

        if (!std::filesystem::exists(sourceConfig)) {
            throw std::runtime_error("Missing source config file: " + sourceConfig.string());
        }

        std::filesystem::create_directories(targetConfig.parent_path());
        std::filesystem::copy_file(sourceConfig, targetConfig, std::filesystem::copy_options::overwrite_existing);
    }

    void ensureAudioAssetsAvailable()
    {
        const std::filesystem::path sourceAudioRoot = std::filesystem::path(testDir_.oldPath()) / "assets/audio";
        const std::filesystem::path targetAudioRoot = "assets/audio";

        if (!std::filesystem::exists(sourceAudioRoot)) {
            throw std::runtime_error("Missing source audio assets folder: " + sourceAudioRoot.string());
        }

        std::filesystem::create_directories(targetAudioRoot);
        std::filesystem::copy(sourceAudioRoot, targetAudioRoot,
                              std::filesystem::copy_options::recursive
                                  | std::filesystem::copy_options::overwrite_existing);
    }

    void clearPersistenceManagerCache() { controller::PersistenceManager::resetConfig(); }

  public:
    TestFixture()
    {
        resetDebugContext();
        ensureGameConfigAvailable();
        ensureAudioAssetsAvailable();
        clearPersistenceManagerCache();
        logger::configure(logger::LogLevel::SILENT, false);
    }
    ~TestFixture() = default;
};