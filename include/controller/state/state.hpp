#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "controller/input/input_state.hpp"
#include "controller/state/state_transition_action.hpp"
#include "controller/view/view.hpp"
#include "game/game.hpp"

namespace controller {

struct BaseState {
    virtual ~BaseState() = default;

    virtual StateTransitionAction update(const InputState &input, float dt) = 0;
    virtual View getView() = 0;
    virtual std::string toString() const = 0;
};

enum class MenuType { MainMenu, PauseMenu, GameOverMenu };

struct MenuState : public BaseState {
    // Add menu-specific state variables here
    MenuType type;
    std::size_t selectedButtonIndex = 0;

    static std::unique_ptr<MenuState> createMainMenu();
    static std::unique_ptr<MenuState> createPauseMenu();
    static std::unique_ptr<MenuState> createGameOverMenu();

    StateTransitionAction update(const InputState &input, float dt) override;
    View getView() override;
    std::string toString() const override;
};

struct GameplayState : public BaseState {
    // Add gameplay-specific state variables here
    game::Game game;

    static std::unique_ptr<GameplayState> createGameplay();

    StateTransitionAction update(const InputState &input, float dt) override;
    View getView() override;
    std::string toString() const override;
};

struct ProgressionStoreState : public BaseState {
    // Add store-specific state variables here

    static std::unique_ptr<ProgressionStoreState> createStore();

    StateTransitionAction update(const InputState &input, float dt) override;
    View getView() override;
    std::string toString() const override;
};

} // namespace controller