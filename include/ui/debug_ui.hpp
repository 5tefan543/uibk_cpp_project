#pragma once

#include "controller/debug/debug_state.hpp"

namespace ui {

class DebugUI {
  public:
    DebugUI();
    ~DebugUI();

    void render(controller::DebugState &debugState, const char *windowTitle = "Debug");
};

} // namespace ui