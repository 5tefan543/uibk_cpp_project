#pragma once

#include "game/ecs/entity.hpp"
#include "game/ecs/registry.hpp"
#include <optional>

namespace game {

struct GameDebugSession {
    // Requests
    bool isStageWaveReloadRequested = false;
    bool isStoreOpenRequested = false;
    bool isPlayerDestructionRequested = false;
    bool isSaveGameRequested = false;

    // stage / wave management
    int stage = 1;
    int wave = 1;

    // ecs management
    Registry &registry;
    std::optional<Entity> selectedEntity;
    bool isSystemUpdateActive = true;

    GameDebugSession(Registry &registry) : registry(registry) {}
};

} // namespace game