#pragma once

#include <string>

namespace controller {

enum class StateType { MainMenu, Gameplay, PauseMenu, ProgressionStore, GameOverMenu };

inline std::string toString(StateType type)
{
    switch (type) {
    case StateType::MainMenu:
        return "MainMenu";
    case StateType::Gameplay:
        return "Gameplay";
    case StateType::PauseMenu:
        return "PauseMenu";
    case StateType::ProgressionStore:
        return "ProgressionStore";
    case StateType::GameOverMenu:
        return "GameOverMenu";
    default:
        return "Unknown";
    }
}

} // namespace controller