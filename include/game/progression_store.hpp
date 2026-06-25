#pragma once

#include "controller/input/input_state.hpp"
#include "game/game.hpp"
#include "view/view.hpp"
#include <deque>
#include <random>

namespace game {

struct StoreItem {
    const StoreItemType type;
    const config::StoreItemConfig &itemConfig;
    const config::StoreItemTypeConfig &typeConfig;
    view::Sprite icon;
    view::Card &card;
    geometry::Rectangle<float> baseCardRect;
};

struct StatsText {
    view::Text &textView;
    std::function<std::string()> getText;
};

enum ButtonTypeId : std::size_t { Buy = 0, Quit, MainMenu, NextStage };

class ProgressionStore {
    Game &game_;
    PlayerStats &playerStats_;
    const config::GameConfig &config_;
    std::mt19937 randomEngine_;

    std::deque<view::Button> buttons_;
    std::deque<view::Card> cards_;
    std::deque<view::Text> texts_;
    std::deque<StatsText> statsTexts_;
    std::deque<StoreItem> storeItems_;

    size_t selectedButtonIndex_ = 0;
    size_t prevSelectedButtonIndex_ = 0;

    view::Card &createBackgroundCard();
    view::Card &createGoldCard();
    view::Card &createPlayerStatsCard();
    view::Card &createStoreItemsCard();
    view::Card &createSelectedItemDetailsCard();
    view::Button &createButton(const geometry::Rectangle<float> &rect, const ButtonTypeId id, const std::string &text);

    void updateButtonSelection();
    void updatePlayerStatsTexts();
    StoreItemType selectStoreItemType(const config::StoreItemConfig &storeItemConfig);

    std::optional<std::size_t> hoveredStoreItemIndex_ = std::nullopt;
    std::optional<std::size_t> selectedStoreItemIndex_ = std::nullopt;

    view::Text *selectedItemNameText_ = nullptr;
    view::Text *selectedItemTypeText_ = nullptr;
    view::Text *selectedItemDescriptionText_ = nullptr;
    view::Text *selectedItemStatChangesText_ = nullptr;
    view::Text *selectedItemCostText_ = nullptr;

    view::Sprite selectedItemIcon_;

    std::optional<std::size_t> getHoveredStoreItemIndex(const controller::InputState &input) const;
    void updateStoreItemSelection(const controller::InputState &input);
    void updateStoreItemViews();
    void updateSelectedItemDetails();

    std::string getStatChangesText(const PlayerStats &statChanges) const;

  public:
    explicit ProgressionStore(Game &game);

    void initView(view::View &view);
    controller::StateTransitionAction update(const controller::InputState &input);
    bool selectedButtonChanged();
};

} // namespace game