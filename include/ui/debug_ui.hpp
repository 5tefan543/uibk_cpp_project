#pragma once

#include "controller/debug/debug_state.hpp"
#include "controller/input/input_state.hpp"

namespace ui {

class DebugUI {
  public:
    DebugUI();
    ~DebugUI();

    float prevFps = 0.0f;

    void render(controller::DebugState &debug, const controller::InputState &input, float fps);
};

} // namespace ui