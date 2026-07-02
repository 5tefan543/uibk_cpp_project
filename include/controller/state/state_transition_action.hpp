#pragma once

namespace controller {

enum class StateTransitionAction {
    None,
    ReplaceCurrentWithCharacterSelection,
    StartNewGameRanged,
    StartNewGameMelee,
    ReplaceCurrentWithLoadedGameplay,
    PushPauseMenu,
    PushProgressionStore,
    ReplaceCurrentWithGameOverMenu,
    Pop,
    ReplaceAllStatesWithMainMenu,
    ReplaceAllStatesWithExit,
};

} // namespace controller