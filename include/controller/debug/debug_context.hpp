#pragma once

#include "controller/debug/game_debug_settings.hpp"
#include "game/debug/game_debug_session.hpp"
#include <string>

namespace controller {

// Contains all debug information that should be preserved across state transitions.
struct DebugContext {
    bool active = false;
    std::string currentStateInfo;
    GameDebugSettings gameSettings;
    game::GameDebugSession *gameSession = nullptr; // only available when Game exists
};

} // namespace controller