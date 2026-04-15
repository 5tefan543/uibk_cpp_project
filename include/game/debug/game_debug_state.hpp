#pragma once

#include "game/ecs/entity.hpp"
#include "game/ecs/registry.hpp"
#include <optional>

namespace game {

struct GameDebugState {
    // stage / wave management
    int stage = 0;
    int wave = 0;
    bool isStageWaveReloadRequested = false;

    // ecs management
    Registry *registry = nullptr;
    std::optional<Entity> selectedEntity;

    // collision
    bool showHitboxes = false;
};

} // namespace game