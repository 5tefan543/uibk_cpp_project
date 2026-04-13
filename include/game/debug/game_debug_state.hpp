#pragma once

#include "game/ecs/entity.hpp"
#include <optional>
#include <string>
#include <vector>

namespace game {

struct GameDebugState {
    // TODO: Refactor that game holds this object???
    // TODO: Reset this object when game is destructed???

    // stage / wave management
    int stage = 0;
    int wave = 0;
    bool isStageWaveReloadRequested = false;

    // ecs management
    std::vector<Entity> entities;
    std::optional<Entity> selectedEntity;

    // collision
    bool showHitboxes = false;
};

} // namespace game