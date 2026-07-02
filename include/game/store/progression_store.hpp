#pragma once

#include "controller/input/input_state.hpp"
#include "game/game.hpp"
#include "view/view.hpp"
#include <deque>
#include <random>

namespace game {

enum ButtonTypeId : std::size_t { Buy = 0, Quit, MainMenu, NextStage };

struct StoreItem {
    const StoreItemType type;
    const config::StoreItemConfig &itemConfig;
    const config::StoreItemTypeConfig &typeConfig;
    view::Sprite icon;
    view::Card &card;
    geometry::Rectangle<float> baseCardRect;
};

struct DynamicText {
    view::Text &textView;
    std::function<std::string()> getText;
};

class ProgressionStore {
    Game &game_;
    PlayerStats &playerStats_;
    const config::GameConfig &config_;
    std::mt19937 randomEngine_;

    std::deque<view::Button> buttons_;
    std::deque<view::Card> cards_;
    std::deque<view::Text> texts_;
    std::deque<DynamicText> dynamicTexts_;
    std::deque<StoreItem> storeItems_;
    view::Sprite selectedItemIcon_;
    view::Button *buyButton_ = nullptr;
    bool buyButtonPressed_ = false;

    size_t selectedButtonIndex_ = 0;
    size_t prevSelectedButtonIndex_ = 0;
    std::optional<std::size_t> prevHoveredStoreItemIndex_ = std::nullopt;
    std::optional<std::size_t> hoveredStoreItemIndex_ = std::nullopt;
    std::optional<std::size_t> prevSelectedStoreItemIndex_ = std::nullopt;
    std::optional<std::size_t> selectedStoreItemIndex_ = std::nullopt;

    view::Card &createBackgroundCard();
    view::Card &createGoldCard();
    view::Card &createPlayerStatsCard();
    view::Card &createStoreItemsCard();
    StoreItemType selectRandomStoreItemType(const config::StoreItemConfig &storeItemConfig);
    view::Card &createSelectedItemDetailsCard();
    const StoreItem *getSelectedStoreItem() const;
    std::string getStatChangesText(const PlayerStats &statChanges) const;
    view::Button &createButton(const geometry::Rectangle<float> &rect, const ButtonTypeId id, const std::string &text);

    bool updateButtonSelection(const controller::InputState &input);
    void updateStoreItemSelection(const controller::InputState &input);
    std::optional<std::size_t> getHoveredStoreItemIndex(const controller::InputState &input) const;
    void updateSelectedItemIcon();
    void updateDynamicTexts();
    void updateStoreItemLayouts();

    bool canBuySelectedStoreItem() const;
    bool buySelectedStoreItem();
    void applyStatChanges(const PlayerStats &statChanges);
    void updateBuyButtonState();

  public:
    explicit ProgressionStore(Game &game);

    void initView(view::View &view);
    controller::StateTransitionAction update(const controller::InputState &input);
    bool selectedButtonChanged() const;
    bool storeItemHoveredChanged() const;
    bool selectedStoreItemChanged() const;
    bool buyButtonPressed() const;
};

} // namespace game