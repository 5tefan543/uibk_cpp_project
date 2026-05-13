#pragma once

#include "controller/debug/debug_context.hpp"
#include "controller/persistence/persistence_manager.hpp"

struct TestFixture {
  private:
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