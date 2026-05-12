#pragma once

namespace controller {

enum class StateTransitionAction {
    None,
    ReplaceCurrentWithGameplay,
    ReplaceCurrentWithLoadedGameplay,
    PushPauseMenu,
    PushProgressionStore,
    ReplaceCurrentWithGameOverMenu,
    Pop,
    ReplaceCurrentWithMainMenu,
    ReplaceAllStatesWithExit,
};

} // namespace controller