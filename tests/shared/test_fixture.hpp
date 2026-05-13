#pragma once

#include "controller/debug/debug_context.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "shared/test_filesystem.hpp"

struct TestFixture {
  private:
    test::ScopedTestDirectory testDir_{"roguelike-game-test-"};

    void resetDebugContext()
    {
        controller::DebugContext &debug = controller::DebugContext::get();
        debug = controller::DebugContext();
    }

    void clearPersistenceManagerCache() { controller::PersistenceManager::resetConfig(); }

  public:
    TestFixture()
    {
        resetDebugContext();
        clearPersistenceManagerCache();
    }
    ~TestFixture() = default;
};