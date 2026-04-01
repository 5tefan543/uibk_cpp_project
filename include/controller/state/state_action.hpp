#pragma once

namespace controller {

enum class StateAction {
    None,
    ReplaceWithGameplay,
    PushPauseMenu,
    PushProgressionStore,
    ReplaceWithGameOverMenu,
    Pop,
    ReplaceWithMainMenu
};

} // namespace controller