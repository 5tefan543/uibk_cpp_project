#pragma once

#include "controller/debug/debug_context.hpp"
#include "controller/input/input_state.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/camera_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "ui/frametime.hpp"
#include "view/sprite.hpp"

namespace ui {

class DebugUI {
  private:
    float prevFps_ = 0.0f;

    void renderStats(const frametimeDelta &dt, const controller::InputState &input, controller::DebugContext &debug);
    void renderGameSettings(controller::DebugContext &debug);
    void renderGameSession(controller::DebugContext &debug);
    void renderEcsManagement(controller::DebugContext &debug, game::GameDebugSession &gameSession);
    void renderLocationTable(controller::DebugContext &debug);
    void renderComponent(game::CameraTag &c);
    void renderComponent(game::Stats &c);
    void renderComponent(game::PlayerStats &c);
    void renderComponent(game::HitBox &c);
    void renderComponent(game::HitBox &c, const std::string &texturePath);
    void renderComponent(game::EnemyStats &c);
    void renderComponent(game::Position &c);
    void renderComponent(game::Velocity &c);
    void renderComponent(game::Animation &c);
    void renderComponent(view::Sprite &c);

  public:
    DebugUI();
    ~DebugUI();

    void render(const controller::InputState &input, const frametimeDelta &dtSec);
};

} // namespace ui