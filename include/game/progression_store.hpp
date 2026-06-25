#pragma once

#include "controller/input/input_state.hpp"
#include "game/game.hpp"
#include "view/view.hpp"

#include <deque>

namespace game {

struct StatsText {
    view::Text &textView;
    std::function<std::string()> getText;
};

enum ButtonTypeId : std::size_t { Buy = 0, Quit, MainMenu, NextStage };

class ProgressionStore {
    Game &game_;
    PlayerStats &playerStats_;

    std::deque<view::Button> buttons_;
    std::deque<view::Card> cards_;
    std::deque<view::Text> texts_;
    std::deque<StatsText> statsTexts_;

    size_t selectedButtonIndex_ = 0;
    size_t prevSelectedButtonIndex_ = 0;
    void updateButtonSelection();

    view::Card &createBackgroundCard();
    view::Card &createGoldCard();
    view::Card &createPlayerStatsCard();
    view::Card &createStoreItemsCard();
    view::Card &createSelectedItemDetailsCard();
    view::Button &createButton(const geometry::Rectangle<float> &rect, const ButtonTypeId id, const std::string &text);

    void updatePlayerStatsTexts();

  public:
    explicit ProgressionStore(Game &game);

    void initView(view::View &view);
    controller::StateTransitionAction update(const controller::InputState &input);
    bool selectedButtonChanged();
};

} // namespace game