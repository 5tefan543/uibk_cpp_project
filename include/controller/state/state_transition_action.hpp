#pragma once

namespace controller {

enum class StateTransitionAction {
    None,
    ReplaceCurrentWithGameplay,
    PushPauseMenu,
    PushProgressionStore,
    ReplaceCurrentWithGameOverMenu,
    Pop,
    ReplaceCurrentWithMainMenu,
    ReplaceAllStatesWithExit,
};

} // namespace controller