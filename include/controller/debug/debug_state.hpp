#pragma once

#include "game/debug/game_debug_state.hpp"
#include <string>

namespace controller {

struct DebugState {
    bool active = false;
    std::string currentStateInfo;
    game::GameDebugState game;
};

} // namespace controller