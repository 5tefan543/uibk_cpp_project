#pragma once

#include "game/ecs/entity.hpp"
#include "game/ecs/registry.hpp"
#include <optional>

namespace game {

struct GameDebugSession {
    // stage / wave management
    bool isStageWaveReloadRequested = false;

    // ecs management
    Registry &registry;
    std::optional<Entity> selectedEntity;

    GameDebugSession(Registry &registry) : registry(registry) {}
};

} // namespace game