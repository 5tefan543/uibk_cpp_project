#include "game/progression_store.hpp"
#include "controller/debug/debug_context.hpp"
#include "controller/input/mouse_util.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "logging/log.hpp"
#include <array>
#include <string>

namespace game {

namespace {
const auto screen = view::grid;
const float screenX = screen.position.x;
const float screenY = screen.position.y;
const float screenW = screen.size.x;
const float screenH = screen.size.y;

constexpr float outerPadding = 40.0f;
constexpr float panelGap = 24.0f;
constexpr float titleHeight = 110.0f;
constexpr float bottomBarHeight = 125.0f;

const float contentX = screenX + outerPadding;
const float contentY = screenY + titleHeight;
const float contentW = screenW - 2.0f * outerPadding;
const float contentH = screenH - titleHeight - bottomBarHeight - outerPadding;

const float statsW = contentW * 0.20f;
const float detailsW = contentW * 0.40f;
const float itemsW = contentW - statsW - detailsW - 2.0f * panelGap;

const float statsX = contentX;
const float itemsX = statsX + statsW + panelGap;
const float detailsX = itemsX + itemsW + panelGap;

const float bottomY = screenY + screenH - bottomBarHeight + 24.0f;
const float bottomButtonH = 70.0f;

constexpr const char *buyButtonText = "BUY";
constexpr const char *quitButtonText = "QUIT";
constexpr const char *mainMenuButtonText = "MAIN MENU";
constexpr const char *nextStageButtonText = "NEXT STAGE";

std::string floatToString(float value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << value;
    return stream.str();
}

geometry::Rectangle<float> scaleRectCentered(const geometry::Rectangle<float> &rect, float scale)
{
    const geometry::Vec2<float> center = rect.getCenter();
    const geometry::Vec2<float> scaledSize = rect.size * scale;

    return geometry::Rectangle<float>{.position = center - scaledSize / 2.0f, .size = scaledSize};
}

} // namespace

ProgressionStore::ProgressionStore(Game &game)
    : game_(game), playerStats_(game.getPlayerStats()), config_(controller::PersistenceManager::getConfig()),
      randomEngine_(std::random_device{}())
{
}

void ProgressionStore::initView(view::View &view)
{
    view::Card &backgroundCard = createBackgroundCard();

    view::Card &goldCard = createGoldCard();
    backgroundCard.elements.push_back(goldCard);

    view::Card &statsCard = createPlayerStatsCard();
    backgroundCard.elements.push_back(statsCard);

    view::Card &itemsCard = createStoreItemsCard();
    backgroundCard.elements.push_back(itemsCard);

    view::Card &detailsCard = createSelectedItemDetailsCard();
    backgroundCard.elements.push_back(detailsCard);

    auto quitButtonRect = geometry::Rectangle<float>{.position = {contentX, bottomY}, .size = {230.0f, bottomButtonH}};
    view::Button &quitButton = createButton(quitButtonRect, ButtonTypeId::Quit, quitButtonText);
    backgroundCard.elements.push_back(quitButton);

    auto mainMenuButtonRect =
        geometry::Rectangle<float>{.position = {contentX + 260.0f, bottomY}, .size = {330.0f, bottomButtonH}};
    view::Button &mainMenuButton = createButton(mainMenuButtonRect, ButtonTypeId::MainMenu, mainMenuButtonText);
    backgroundCard.elements.push_back(mainMenuButton);

    auto nextStageButtonRect = geometry::Rectangle<float>{
        .position = {screenX + screenW - outerPadding - 420.0f, bottomY}, .size = {420.0f, bottomButtonH}};
    view::Button &nextStageButton = createButton(nextStageButtonRect, ButtonTypeId::NextStage, nextStageButtonText);
    backgroundCard.elements.push_back(nextStageButton);

    view.nodes.push_back({view::ViewMode::FixedToScreen, backgroundCard});
    updateButtonSelection();
}

view::Card &ProgressionStore::createBackgroundCard()
{
    view::Card &backgroundCard = cards_.emplace_back(view::Card());
    backgroundCard.rect = screen;
    backgroundCard.backgroundColor = view::Color{12, 12, 12};

    view::Text &title = texts_.emplace_back(view::Text());
    title.text = "PROGRESSION STORE";
    title.position = {screenX + screenW * 0.5f, screenY + 62.0f};

    backgroundCard.elements.push_back(title);
    return backgroundCard;
}

view::Card &ProgressionStore::createGoldCard()
{
    // gold card top right corner
    view::Card &goldCard = cards_.emplace_back(view::Card());
    goldCard.rect = geometry::Rectangle<float>{.position = {screenX + screenW - outerPadding - 300.0f, screenY + 24.0f},
                                               .size = {300.0f, 68.0f}};
    goldCard.backgroundColor = view::Color{25, 25, 25};

    view::Text &goldText = texts_.emplace_back(view::Text());
    goldText.position = goldCard.rect.getCenter();

    statsTexts_.push_back(StatsText{
        .textView = goldText,
        .getText = [this]() { return "Gold: " + std::to_string(playerStats_.currency); },
    });

    goldCard.elements.push_back(goldText);

    return goldCard;
}

view::Card &ProgressionStore::createPlayerStatsCard()
{
    view::Card &statsCard = cards_.emplace_back(view::Card());
    statsCard.rect = geometry::Rectangle<float>{.position = {statsX, contentY}, .size = {statsW, contentH}};
    statsCard.backgroundColor = view::Color{28, 28, 28};

    view::Text &statsTitle = texts_.emplace_back(view::Text());
    statsTitle.text = "PLAYER STATS";
    statsTitle.position = {statsCard.rect.getCenter().x, contentY + 45.0f};
    statsTitle.alignment = view::TextAlignment::Center;

    statsCard.elements.push_back(statsTitle);

    float statY = contentY + 105.0f;

    const float rowPadding = 24.0f;
    const float labelX = statsCard.rect.position.x + rowPadding;
    const float valueX = statsCard.rect.position.x + statsCard.rect.size.x - rowPadding;

    constexpr unsigned int statTextSize = 22;
    constexpr float rowSpacing = 50.0f;

    auto addStatsRow = [this, &statsCard, &statY, labelX, valueX](const std::string &label,
                                                                  std::function<std::string()> getValueText) {
        view::Text &labelText = texts_.emplace_back(view::Text());
        labelText.text = label;
        labelText.position = {labelX, statY};
        labelText.alignment = view::TextAlignment::Left;
        labelText.size = statTextSize;

        view::Text &valueText = texts_.emplace_back(view::Text());
        valueText.position = {valueX, statY};
        valueText.alignment = view::TextAlignment::Right;
        valueText.size = statTextSize;

        statsTexts_.push_back(StatsText{
            .textView = valueText,
            .getText = getValueText,
        });

        statsCard.elements.push_back(labelText);
        statsCard.elements.push_back(valueText);

        statY += rowSpacing;
    };

    addStatsRow("Max Health", [this]() { return floatToString(playerStats_.maxHealth); });
    addStatsRow("Attack", [this]() { return floatToString(playerStats_.attackPower); });
    addStatsRow("Attack Speed", [this]() { return floatToString(playerStats_.attackSpeed); });
    addStatsRow("Defense", [this]() { return floatToString(playerStats_.defense); });
    addStatsRow("Move Speed", [this]() { return floatToString(playerStats_.moveSpeed); });
    addStatsRow("Speed of Attack", [this]() { return floatToString(playerStats_.speedOfAttack); });
    addStatsRow("Attack Range", [this]() { return floatToString(playerStats_.attackRange); });
    addStatsRow("Dash", [this]() { return std::string(playerStats_.hasDash ? "Yes" : "No"); });

    return statsCard;
}

view::Card &ProgressionStore::createStoreItemsCard()
{
    view::Card &itemsCard = cards_.emplace_back(view::Card());
    itemsCard.rect = geometry::Rectangle<float>{.position = {itemsX, contentY}, .size = {itemsW, contentH}};
    itemsCard.backgroundColor = view::Color{22, 22, 22};

    view::Text &itemsTitle = texts_.emplace_back(view::Text());
    itemsTitle.text = "UPGRADES";
    itemsTitle.position = {itemsCard.rect.getCenter().x, contentY + 45.0f};

    itemsCard.elements.push_back(itemsTitle);

    constexpr int itemColumns = 3;
    constexpr float itemSize = 80.0f;
    constexpr float itemGapX = 100.0f;
    constexpr float itemGapY = 100.0f;
    constexpr float iconPadding = 8.0f;

    const int itemCount = static_cast<int>(config_.storeConfig.items.size());
    const int visibleColumns = std::min(itemColumns, std::max(itemCount, 1));

    const float totalItemGridW = visibleColumns * itemSize + (visibleColumns - 1) * itemGapX;
    const float itemGridStartX = itemsX + (itemsW - totalItemGridW) * 0.5f;
    const float itemGridStartY = contentY + 135.0f;

    int visibleItemIndex = 0;

    for (const config::StoreItemConfig &storeItemConfig : config_.storeConfig.items) {
        if (storeItemConfig.typeToConfig.empty()) {
            logger::log(logger::LogLevel::ERROR,
                        std::format("Store item {} has no type configurations.", storeItemConfig.name));
            continue;
        }

        const StoreItemType selectedType = selectStoreItemType(storeItemConfig);
        const config::StoreItemTypeConfig &typeConfig = storeItemConfig.typeToConfig.at(selectedType);

        const int row = visibleItemIndex / itemColumns;
        const int col = visibleItemIndex % itemColumns;

        const float itemX = itemGridStartX + col * (itemSize + itemGapX);
        const float itemY = itemGridStartY + row * (itemSize + itemGapY);

        view::Card &itemCard = cards_.emplace_back(view::Card());
        itemCard.rect = geometry::Rectangle<float>{.position = {itemX, itemY}, .size = {itemSize, itemSize}};
        itemCard.backgroundColor = view::Color{55, 45, 55};

        const std::size_t itemIndex = storeItems_.size();

        StoreItem &storeItem = storeItems_.emplace_back(StoreItem{
            .type = selectedType,
            .itemConfig = storeItemConfig,
            .typeConfig = typeConfig,
            .icon =
                view::Sprite{.rect = geometry::Rectangle<float>{.position = {itemX + iconPadding, itemY + iconPadding},
                                                                .size = {itemSize - 2.0f * iconPadding,
                                                                         itemSize - 2.0f * iconPadding}},
                             .imagePath = typeConfig.icon.path},
            .card = itemCard,
            .baseCardRect = itemCard.rect});

        itemCard.elements.push_back(storeItem.icon);
        itemsCard.elements.push_back(itemCard);

        if (!selectedStoreItemIndex_.has_value()) {
            selectedStoreItemIndex_ = itemIndex;
        }

        ++visibleItemIndex;
    }

    return itemsCard;
}

view::Card &ProgressionStore::createSelectedItemDetailsCard()
{
    view::Card &detailsCard = cards_.emplace_back(view::Card());
    detailsCard.rect = geometry::Rectangle<float>{.position = {detailsX, contentY}, .size = {detailsW, contentH}};
    detailsCard.backgroundColor = view::Color{28, 28, 28};

    selectedItemNameText_ = &texts_.emplace_back(view::Text());
    selectedItemNameText_->position = {detailsCard.rect.getCenter().x, contentY + 55.0f};
    selectedItemNameText_->size = 28;
    detailsCard.elements.push_back(*selectedItemNameText_);

    view::Card &selectedItemCard = cards_.emplace_back(view::Card());
    selectedItemCard.rect = geometry::Rectangle<float>{
        .position = {detailsCard.rect.getCenter().x - 48.0f, contentY + 110.0f}, .size = {96.0f, 96.0f}};
    selectedItemCard.backgroundColor = view::Color{55, 45, 55};

    selectedItemIcon_.rect = geometry::Rectangle<float>{
        .position = {selectedItemCard.rect.position.x + 8.0f, selectedItemCard.rect.position.y + 8.0f},
        .size = {80.0f, 80.0f}};

    selectedItemCard.elements.push_back(selectedItemIcon_);
    detailsCard.elements.push_back(selectedItemCard);

    selectedItemTypeText_ = &texts_.emplace_back(view::Text());
    selectedItemTypeText_->position = {detailsCard.rect.getCenter().x, contentY + 235.0f};
    selectedItemTypeText_->size = 22;

    selectedItemDescriptionText_ = &texts_.emplace_back(view::Text());
    selectedItemDescriptionText_->position = {detailsCard.rect.getCenter().x, contentY + 290.0f};
    selectedItemDescriptionText_->size = 20;

    selectedItemStatChangesText_ = &texts_.emplace_back(view::Text());
    selectedItemStatChangesText_->position = {detailsCard.rect.getCenter().x, contentY + 365.0f};
    selectedItemStatChangesText_->size = 20;

    selectedItemCostText_ = &texts_.emplace_back(view::Text());
    selectedItemCostText_->position = {detailsCard.rect.getCenter().x, contentY + 455.0f};
    selectedItemCostText_->size = 22;

    detailsCard.elements.push_back(*selectedItemTypeText_);
    detailsCard.elements.push_back(*selectedItemDescriptionText_);
    detailsCard.elements.push_back(*selectedItemStatChangesText_);
    detailsCard.elements.push_back(*selectedItemCostText_);

    auto buyButtonRect = geometry::Rectangle<float>{.position = {detailsX + 36.0f, contentY + contentH - 92.0f},
                                                    .size = {detailsW - 72.0f, 64.0f}};

    view::Button &buyButton = createButton(buyButtonRect, ButtonTypeId::Buy, buyButtonText);
    detailsCard.elements.push_back(buyButton);

    return detailsCard;
}

view::Button &ProgressionStore::createButton(const geometry::Rectangle<float> &rect, const ButtonTypeId id,
                                             const std::string &text)
{
    view::Button &button = buttons_.emplace_back(view::Button());
    button.id = id;
    button.rect = rect;
    button.text.text = text;
    button.text.position = button.rect.getCenter();
    return button;
}

controller::StateTransitionAction ProgressionStore::update(const controller::InputState &input)
{
    updateStoreItemSelection(input);

    prevSelectedButtonIndex_ = selectedButtonIndex_;

    controller::StateTransitionAction stateTransitionAction = controller::StateTransitionAction::None;

    const bool isMouseSelectionActive = input.mouseMoved || input.mouseLeftPressed;

    const std::optional<std::size_t> hoveredButtonIndex = controller::MouseUtil::getHoveredButtonId(input, buttons_);

    if (isMouseSelectionActive && hoveredButtonIndex.has_value()) {
        selectedButtonIndex_ = hoveredButtonIndex.value();
    }

    const bool isButtonHovered = hoveredButtonIndex.has_value();
    const bool buttonPressed = input.confirmPressed || (input.mouseLeftPressed && isButtonHovered);

    controller::DebugContext &debug = controller::DebugContext::get();

    if (buttonPressed) {
        const view::Button &selectedButton = buttons_[selectedButtonIndex_];

        switch (selectedButton.id) {
        case ButtonTypeId::Buy:
            // TODO: buy selected upgrade
            break;

        case ButtonTypeId::Quit:
            stateTransitionAction = controller::StateTransitionAction::ReplaceAllStatesWithExit;
            debug.gameSession = nullptr;
            break;

        case ButtonTypeId::MainMenu:
            stateTransitionAction = controller::StateTransitionAction::ReplaceAllStatesWithMainMenu;
            debug.gameSession = nullptr;
            break;

        case ButtonTypeId::NextStage:
            stateTransitionAction = controller::StateTransitionAction::Pop;
            break;

        default:
            break;
        }
    }

    updateButtonSelection();
    updatePlayerStatsTexts();
    updateStoreItemViews();
    updateSelectedItemDetails();

    return stateTransitionAction;
}

void ProgressionStore::updateButtonSelection()
{
    for (std::size_t i = 0; i < buttons_.size(); ++i) {
        buttons_[i].isSelected = i == selectedButtonIndex_;
    }
}

void ProgressionStore::updatePlayerStatsTexts()
{
    for (StatsText &statsText : statsTexts_) {
        statsText.textView.text = statsText.getText();
    }
}

bool ProgressionStore::selectedButtonChanged()
{
    return selectedButtonIndex_ != prevSelectedButtonIndex_;
}

StoreItemType ProgressionStore::selectStoreItemType(const config::StoreItemConfig &storeItemConfig)
{
    std::vector<StoreItemType> availableTypes;
    std::vector<float> weights;

    for (const auto &[type, typeConfig] : storeItemConfig.typeToConfig) {
        availableTypes.push_back(type);
        weights.push_back(config_.storeConfig.typeToRandomWeight.at(type));
    }

    std::discrete_distribution<std::size_t> distribution(weights.begin(), weights.end());
    return availableTypes[distribution(randomEngine_)];
}

std::optional<std::size_t> ProgressionStore::getHoveredStoreItemIndex(const controller::InputState &input) const
{
    for (std::size_t i = 0; i < storeItems_.size(); ++i) {
        if (storeItems_[i].baseCardRect.contains(input.mouseGrid)) {
            return i;
        }
    }

    return std::nullopt;
}

void ProgressionStore::updateStoreItemSelection(const controller::InputState &input)
{
    if (!input.mouseMoved && !input.mouseLeftPressed) {
        return;
    }

    hoveredStoreItemIndex_ = getHoveredStoreItemIndex(input);

    if (input.mouseLeftPressed && hoveredStoreItemIndex_.has_value()) {
        selectedStoreItemIndex_ = hoveredStoreItemIndex_;
        updateSelectedItemDetails();
    }
}

void ProgressionStore::updateStoreItemViews()
{
    constexpr float iconPadding = 8.0f;

    for (std::size_t i = 0; i < storeItems_.size(); ++i) {
        StoreItem &item = storeItems_[i];

        const bool isHovered = hoveredStoreItemIndex_.has_value() && hoveredStoreItemIndex_.value() == i;
        const bool isSelected = selectedStoreItemIndex_.has_value() && selectedStoreItemIndex_.value() == i;

        float scale = 1.0f;

        if (isHovered) {
            scale = 2.0f;
        } else if (isSelected) {
            scale = 1.5f;
        }

        item.card.rect = scaleRectCentered(item.baseCardRect, scale);

        item.icon.rect = geometry::Rectangle<float>{
            .position = {item.card.rect.position.x + iconPadding, item.card.rect.position.y + iconPadding},
            .size = {item.card.rect.size.x - 2.0f * iconPadding, item.card.rect.size.y - 2.0f * iconPadding}};

        if (isSelected) {
            item.card.backgroundColor = view::Color{90, 70, 110};
        } else if (isHovered) {
            item.card.backgroundColor = view::Color{75, 60, 90};
        } else {
            item.card.backgroundColor = view::Color{55, 45, 55};
        }
    }
}

void ProgressionStore::updateSelectedItemDetails()
{
    if (!selectedStoreItemIndex_.has_value() || selectedStoreItemIndex_.value() >= storeItems_.size()) {
        if (selectedItemNameText_ != nullptr) {
            selectedItemNameText_->text = "No item selected";
        }

        return;
    }

    const StoreItem &item = storeItems_[selectedStoreItemIndex_.value()];

    if (selectedItemNameText_ != nullptr) {
        selectedItemNameText_->text = item.itemConfig.name;
    }

    if (selectedItemTypeText_ != nullptr) {
        selectedItemTypeText_->text = toString(item.type);
    }

    if (selectedItemDescriptionText_ != nullptr) {
        selectedItemDescriptionText_->text = item.itemConfig.description;
    }

    if (selectedItemStatChangesText_ != nullptr) {
        selectedItemStatChangesText_->text = getStatChangesText(item.typeConfig.statChanges);
    }

    if (selectedItemCostText_ != nullptr) {
        selectedItemCostText_->text = "Cost: " + floatToString(item.typeConfig.cost) + " Gold";
    }

    selectedItemIcon_.imagePath = item.typeConfig.icon.path;
}

std::string ProgressionStore::getStatChangesText(const PlayerStats &statChanges) const
{
    std::vector<std::string> changes;

    auto addFloatChange = [&changes](const std::string &label, float value) {
        if (value != 0.0f) {
            const std::string sign = value > 0.0f ? "+" : "-";
            changes.push_back(label + ": " + sign + floatToString(value));
        }
    };

    auto addIntChange = [&changes](const std::string &label, int value) {
        if (value != 0) {
            const std::string sign = value > 0 ? "+" : "-";
            changes.push_back(label + ": " + sign + std::to_string(value));
        }
    };

    addFloatChange("Max Health", statChanges.maxHealth);
    addFloatChange("Attack", statChanges.attackPower);
    addFloatChange("Attack Speed", statChanges.attackSpeed);
    addFloatChange("Defense", statChanges.defense);
    addFloatChange("Move Speed", statChanges.moveSpeed);
    addFloatChange("Speed of Attack", statChanges.speedOfAttack);
    addFloatChange("Attack Range", statChanges.attackRange);
    addIntChange("Pierce", statChanges.enemiesPierced);

    if (statChanges.hasDash) {
        changes.push_back("Dash: unlocked");
    }

    if (changes.empty()) {
        return "No stat changes";
    }

    std::string result;

    for (std::size_t i = 0; i < changes.size(); ++i) {
        result += changes[i];

        if (i + 1 < changes.size()) {
            result += "\n";
        }
    }

    return result;
}

} // namespace game