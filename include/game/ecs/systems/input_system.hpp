#pragma once

#include "controller/input/input_state.hpp"
#include "game/ecs/registry.hpp"

namespace game {

struct InputSystem {
    void update(Registry &registry, const controller::InputState &input);
};

} // namespace game