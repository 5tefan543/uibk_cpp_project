#pragma once

#include "controller/debug/debug_state.hpp"
#include "controller/input/input_state.hpp"

namespace ui {

class DebugUI {
  public:
    DebugUI();
    ~DebugUI();

    float prevFPS = 0.0f;

    void render(controller::DebugState &debugState, const controller::InputState &inputState, float fps);
};

} // namespace ui