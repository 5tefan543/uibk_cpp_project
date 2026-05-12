#pragma once

#include "controller/input/input_state.hpp"
#include "game/debug/game_debug_session.hpp"
#include "game/ecs/registry.hpp"

namespace game {

struct DebugSelectionSystem {
    void update(Registry &registry, const controller::InputState &input, bool isDebugActive,
                game::GameDebugSession &debugSession);

  private:
    void clearSelection(Registry &registry, GameDebugSession &debugSession);
    std::optional<Entity> getEntityAtMousePosition(Registry &registry, const controller::InputState &input);
    void updateSelection(Registry &registry, GameDebugSession &debugSession);
};

} // namespace game