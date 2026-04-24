#pragma once

#include "controller/debug/debug_context.hpp"
#include "controller/input/input_state.hpp"
#include "game/ecs/components/camera.hpp"
#include "game/ecs/components/map.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sprite.hpp"
#include "game/ecs/components/velocity.hpp"

namespace ui {

class DebugUI {
  private:
    float prevFps_ = 0.0f;

    void renderStats(float fps, const controller::InputState &input, controller::DebugContext &debug);
    void renderGameSettings(controller::DebugContext &debug);
    void renderGameSession(controller::DebugContext &debug);
    void renderEcsManagement(game::GameDebugSession &gameSession);
    void renderComponent(game::PlayerTag &c);
    void renderComponent(game::Position &c);
    void renderComponent(game::Velocity &c);
    void renderComponent(game::Sprite &c);
    void renderComponent(game::Map &c);
    void renderComponent(game::Camera &c);

  public:
    DebugUI();
    ~DebugUI();

    void render(controller::DebugContext &debug, const controller::InputState &input, float fps);
};

} // namespace ui