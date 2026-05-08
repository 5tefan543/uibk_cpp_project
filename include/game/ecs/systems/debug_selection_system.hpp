#pragma once

#include "controller/debug/debug_context.hpp"
#include "controller/input/input_state.hpp"
#include "game/ecs/registry.hpp"

namespace game {

struct DebugSelectionSystem {
    void update(Registry &registry, const controller::InputState &input, bool isDebugActive,
                game::GameDebugSession &debugSession);
};

} // namespace game