#pragma once

namespace controller {

// Contains all game debug settings that should be preserved across game instances.
struct GameDebugSettings {
    // stage / wave management
    int stage = 0;
    int wave = 0;

    // collision
    bool showHitboxes = false;
};

} // namespace controller