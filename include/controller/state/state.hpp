#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "controller/debug/debug_context.hpp"
#include "controller/input/input_state.hpp"
#include "controller/state/state_transition_action.hpp"
#include "controller/view/view.hpp"
#include "game/game.hpp"

namespace controller {

struct BaseState {
    virtual ~BaseState() = default;

    virtual StateTransitionAction update(const InputState &input, DebugContext &debug, float dt) = 0;
    virtual const View &getView();
    virtual std::string toString() const = 0;

  protected:
    View view_;
};

enum class MenuType { MainMenu, PauseMenu, GameOverMenu };

struct MenuState : public BaseState {
    // Add menu-specific state variables here
    const MenuType type;

    static std::unique_ptr<MenuState> createMenu(const MenuType menuType);

    StateTransitionAction update(const InputState &input, DebugContext &debug, float dt) override;
    std::string toString() const override;

  private:
    std::size_t selectedButtonID_ = 0;
    std::vector<std::shared_ptr<Button>> buttons_; // Shares Button with view_.items

    MenuState(MenuType type);
    void initView();
};

struct GameplayState : public BaseState {
    // Add gameplay-specific state variables here
    game::Game game;

    static std::unique_ptr<GameplayState> createGameplay();

    StateTransitionAction update(const InputState &input, DebugContext &debug, float dt) override;
    std::string toString() const override;
};

struct ProgressionStoreState : public BaseState {
    // Add store-specific state variables here

    static std::unique_ptr<ProgressionStoreState> createStore();

    StateTransitionAction update(const InputState &input, DebugContext &debug, float dt) override;
    std::string toString() const override;
};

struct ExitState : public BaseState {

    static std::unique_ptr<ExitState> createExitState();

    StateTransitionAction update(const InputState &input, DebugContext &debug, float dt) override;
    std::string toString() const override;
};

} // namespace controller