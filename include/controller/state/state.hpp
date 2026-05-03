#pragma once

#include <cstdint>
#include <deque>
#include <memory>
#include <string>

#include "controller/debug/debug_context.hpp"
#include "controller/input/input_state.hpp"
#include "controller/state/state_transition_action.hpp"
#include "controller/view/view.hpp"
#include "game/game.hpp"
#include <optional>

namespace controller {

class BaseState {
  protected:
    View view_;

  public:
    virtual ~BaseState() = default;

    virtual StateTransitionAction update(const InputState &input, DebugContext &debug, float dt) = 0;
    virtual const View &getView();
    virtual std::string toString() const = 0;
};

enum class MenuType { MainMenu, PauseMenu, GameOverMenu };

class MenuState : public BaseState {
    std::deque<Button> buttons_;
    std::deque<Card> cards_;
    std::deque<Text> texts_;
    std::size_t selectedButtonId_ = 0;

    MenuState(MenuType type);
    void initView();
    std::optional<std::size_t> getHoveredButtonId(const InputState &input) const;

  public:
    const MenuType type;
    static std::unique_ptr<MenuState> createMenu(const MenuType menuType);

    StateTransitionAction update(const InputState &input, DebugContext &debug, float dt) override;
    std::string toString() const override;
};

class GameplayState : public BaseState {
    bool loadedFromSave_ = false;

  public:
    game::Game game;

    static std::unique_ptr<GameplayState> createGameplay();
    static std::unique_ptr<GameplayState> createLoadedGameplay();
    bool didLoadFromSave() const;

    StateTransitionAction update(const InputState &input, DebugContext &debug, float dt) override;
    std::string toString() const override;
    const View &getView() override;
};

class ProgressionStoreState : public BaseState {
  public:
    static std::unique_ptr<ProgressionStoreState> createStore();

    StateTransitionAction update(const InputState &input, DebugContext &debug, float dt) override;
    std::string toString() const override;
};

class ExitState : public BaseState {
  public:
    static std::unique_ptr<ExitState> createExitState();

    StateTransitionAction update(const InputState &input, DebugContext &debug, float dt) override;
    std::string toString() const override;
};

} // namespace controller