#pragma once

#include "controller/input/input_state.hpp"
#include "game/game.hpp"
#include "view/view.hpp"
#include <deque>

namespace game {

class ProgressionStore {
    const Game &game;
    std::deque<view::Button> buttons_;
    std::deque<view::Card> cards_;
    std::deque<view::Text> texts_;
    std::size_t selectedButtonId_ = 0;
    std::size_t prevSelectedButtonId_ = 0;

  public:
    explicit ProgressionStore(const Game &game);
    void initView(view::View &view);
    controller::StateTransitionAction update(const controller::InputState &input);
    void updateView(view::View &view);
    bool selectedButtonChanged();
};

} // namespace game