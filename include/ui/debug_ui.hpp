#pragma once

#include "controller/debug/debug_context.hpp"
#include "controller/input/input_state.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"

namespace ui {

class DebugUI {
  private:
    float prevFps = 0.0f;

    void renderComponent(game::PlayerTag &c);
    void renderComponent(game::Position &c);
    void renderComponent(game::Velocity &c);

  public:
    DebugUI();
    ~DebugUI();

    void render(controller::DebugContext &debug, const controller::InputState &input, float fps);
};

} // namespace ui