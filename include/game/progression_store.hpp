#pragma once

#include "controller/input/input_state.hpp"
#include "game/game.hpp"
#include "view/view.hpp"

#include <deque>

namespace game {

enum ButtonId { Buy, Quit, MainMenu, NextStage };

class ProgressionStore {
    const Game &game;

    std::deque<view::Button> buttons_;
    std::deque<view::Card> cards_;
    std::deque<view::Text> texts_;

    size_t selectedButtonId_ = ButtonId::MainMenu;
    size_t prevSelectedButtonId_ = ButtonId::MainMenu;
    void updateButtonSelection();

    view::Card &createBackgroundCard(view::View &view);
    view::Card &createGoldCard(view::View &view);
    view::Card &createPlayerStatsCard(view::View &view);
    view::Card &createStoreItemsCard(view::View &view);
    view::Card &createSelectedItemDetailsCard(view::View &view);
    view::Button &createButton(view::View &view, const geometry::Rectangle<float> &rect, const std::string &text,
                               ButtonId buttonId);

  public:
    explicit ProgressionStore(const Game &game);

    void initView(view::View &view);
    controller::StateTransitionAction update(const controller::InputState &input);
    void updateView(view::View &view);

    bool selectedButtonChanged();
};

} // namespace game