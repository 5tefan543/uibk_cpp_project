#pragma once

#include <chrono>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <array>
#include <vector>

#include "controller/input/input_state.hpp"
#include "controller/state/state_transition_action.hpp"
#include "game/game.hpp"
#include "controller/persistence/leaderboard.hpp"
#include "view/view.hpp"

namespace controller {

class BaseState {
  protected:
    view::View view_;

  public:
    virtual ~BaseState() = default;

    virtual StateTransitionAction update(const InputState &input, float dt) = 0;
    virtual const view::View &getView();
    virtual std::string toString() const = 0;
};

enum class MenuType { MainMenu, PauseMenu, GameOverMenu, CharacterSelection };

class MenuState : public BaseState {
    std::deque<view::Button> buttons_;
    std::deque<view::Card> cards_;
    std::deque<view::Text> texts_;
    std::size_t selectedButtonId_ = 0;
    std::size_t prevSelectedButtonId_ = 0;
  MenuState(const game::Game &game);
  MenuState(MenuType type);
    void initView();

  struct GameOverData {
    int score = 0;
    int wave = 0;
    std::vector<controller::LeaderboardEntry> leaderboardEntries;
    int playerPosition = -1; // 1-based position
    bool nameSubmitted = false;
    std::string nameBuffer; // up to 25 chars
    std::size_t nameSelectedIndex = 0;
    // indices into texts_ for dynamic updates
    std::size_t leaderboardTextStartIndex = 0;
    std::size_t leaderboardTextCount = 0;
    std::size_t nameTextIndex = 0;
  };

  std::optional<GameOverData> gameOverData_;

  public:
    const MenuType type;
    static std::unique_ptr<MenuState> createMenu(const MenuType menuType);
    static std::unique_ptr<MenuState> createMenu(const game::Game &game);
    bool selectedButtonChanged();
    StateTransitionAction update(const InputState &input, float dt) override;
    std::string toString() const override;
};

class GameplayState : public BaseState {
    bool loadedFromSave_ = false;
    int currentWave_ = 1;

    explicit GameplayState(game::CharacterType characterType);
    explicit GameplayState(const game::PersistedGame &persistedGame);

  public:
    game::Game game;
    static std::unique_ptr<GameplayState> createNewGameplay(game::CharacterType characterType);
    static std::unique_ptr<GameplayState> createLoadedGameplay();
    bool isLoadedFromPersistedGame() const;
    bool hasWaveChanged();
    StateTransitionAction update(const InputState &input, float dt) override;
    std::string toString() const override;
    const view::View &getView() override;
};

class ProgressionStoreState : public BaseState {
    std::deque<view::Button> buttons_;
    std::deque<view::Card> cards_;
    std::deque<view::Text> texts_;
    std::size_t selectedButtonId_ = 0;
    std::size_t prevSelectedButtonId_ = 0;

    ProgressionStoreState();
    void initView();

  public:
    static std::unique_ptr<ProgressionStoreState> createStore();
    bool selectedButtonChanged();
    StateTransitionAction update(const InputState &input, float dt) override;
    std::string toString() const override;
};

class ExitState : public BaseState {
  public:
    static std::unique_ptr<ExitState> createExitState();
    StateTransitionAction update(const InputState &input, float dt) override;
    std::string toString() const override;
};

} // namespace controller