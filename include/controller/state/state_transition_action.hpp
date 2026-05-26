#pragma once

namespace controller {

enum class StateTransitionAction {
    None,
    ReplaceCurrentWithCharacterSelection,
    StartNewGameRanged,
    StartNewGameMelee,
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