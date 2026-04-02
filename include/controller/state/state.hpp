#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "controller/input/input_state.hpp"
#include "controller/presentation/button.hpp"
#include "controller/presentation/color.hpp"
#include "controller/state/state_transition_action.hpp"
#include "controller/state/state_type.hpp"
#include "game/game.hpp"

namespace controller {

struct BaseState {
    StateType type;
    Color backgroundColor;

    virtual ~BaseState() = default;

    virtual StateTransitionAction update(const InputState &input, float dt) = 0;
};

struct MenuState : public BaseState {
    // Add menu-specific state variables here
    std::string title;
    std::vector<Button> buttons;
    std::size_t selectedButtonIndex = 0;

    static std::unique_ptr<MenuState> createMainMenu();
    static std::unique_ptr<MenuState> createPauseMenu();
    static std::unique_ptr<MenuState> createGameOverMenu();

    StateTransitionAction update(const InputState &input, float dt) override;
};

struct GameplayState : public BaseState {
    // Add gameplay-specific state variables here
    game::Game game;

    static std::unique_ptr<GameplayState> createGameplay();

    StateTransitionAction update(const InputState &input, float dt) override;
};

struct ProgressionStoreState : public BaseState {
    // Add store-specific state variables here

    static std::unique_ptr<ProgressionStoreState> createStore();

    StateTransitionAction update(const InputState &input, float dt) override;
};

} // namespace controller