#pragma once

namespace controller {

// Contains all game debug settings that should be preserved across game instances.
struct GameDebugSettings {

    // collision
    bool showHitboxes = false;
    bool showLocationTable = false;
};

} // namespace controller