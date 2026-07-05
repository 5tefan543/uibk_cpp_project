#include "game/store/progression_store.hpp"
#include "controller/debug/debug_context.hpp"
#include "controller/input/mouse_util.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "game/store/progression_store_helper.hpp"
#include "logging/log.hpp"
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

constexpr view::Color backgroundCardColor = view::color::nearBlack;
constexpr view::Color goldCardColor = view::color::veryDarkGray;
constexpr view::Color statsCardColor = view::color::charcoal;
constexpr view::Color itemsCardColor = view::color::almostBlack;
constexpr view::Color detailsCardColor = view::color::charcoal;

constexpr view::Color storeItemCardColor = view::color::darkMutedPurple;
constexpr view::Color hoveredStoreItemCardColor = view::color::mutedPurple;
constexpr view::Color selectedStoreItemCardColor = view::color::lightMutedPurple;

constexpr view::Color buyButtonColor = view::color::gray;
constexpr view::Color buyButtonSelectedColor = view::color::green;
constexpr view::Color disabledBuyButtonColor = view::color::darkGray;
constexpr view::Color disabledBuyButtonSelectedColor = view::color::dimGray;
constexpr view::Color disabledBuyButtonTextColor = view::color::lightGray;
constexpr view::Color enabledBuyButtonTextColor = view::color::white;

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
}

controller::StateTransitionAction ProgressionStore::update(const controller::InputState &input)
{
    buyButtonPressed_ = false;
    bool buttonPressed = updateButtonSelection(input);
    updateStoreItemSelection(input);

    controller::StateTransitionAction stateTransitionAction = controller::StateTransitionAction::None;
    controller::DebugContext &debug = controller::DebugContext::get();

    if (buttonPressed) {
        const view::Button &selectedButton = buttons_[selectedButtonIndex_];

        switch (selectedButton.id) {
        case ButtonTypeId::Buy:
            buySelectedStoreItem();
            game_.resetPlayerHealth(); // reset player health after possible max health increase
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
            game_.setShouldOpenStore(false);
            game_.save();
            controller::PersistenceManager::deleteStore();
            break;

        default:
            break;
        }
    }

    updateDynamicTexts();
    updateStoreItemLayouts();
    updateSelectedItemIcon();
    updateBuyButtonState();

    return stateTransitionAction;
}

bool ProgressionStore::selectedButtonChanged() const
{
    return selectedButtonIndex_ != prevSelectedButtonIndex_;
}

bool ProgressionStore::storeItemHoveredChanged() const
{
    return hoveredStoreItemIndex_ != prevHoveredStoreItemIndex_;
}

bool ProgressionStore::selectedStoreItemChanged() const
{
    return selectedStoreItemIndex_ != prevSelectedStoreItemIndex_;
}

bool ProgressionStore::buyButtonPressed() const
{
    return buyButtonPressed_;
}

view::Card &ProgressionStore::createBackgroundCard()
{
    view::Card &backgroundCard = cards_.emplace_back(view::Card());
    backgroundCard.rect = screen;
    backgroundCard.backgroundColor = backgroundCardColor;

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
    goldCard.backgroundColor = goldCardColor;

    view::Text &goldText = texts_.emplace_back(view::Text());
    goldText.position = goldCard.rect.getCenter();

    dynamicTexts_.push_back(DynamicText{
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
    statsCard.backgroundColor = statsCardColor;

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

        dynamicTexts_.push_back(DynamicText{
            .textView = valueText,
            .getText = getValueText,
        });

        statsCard.elements.push_back(labelText);
        statsCard.elements.push_back(valueText);

        statY += rowSpacing;
    };

    addStatsRow("Max Health", [this]() { return floatToPrettyString(playerStats_.maxHealth); });
    addStatsRow("Attack", [this]() { return floatToPrettyString(playerStats_.attackPower); });
    addStatsRow("Attack Speed", [this]() { return floatToPrettyString(playerStats_.attackSpeed); });
    addStatsRow("Special Attack Speed", [this]() { return floatToPrettyString(playerStats_.specialAttackSpeed); });
    addStatsRow("Defense", [this]() { return floatToPrettyString(playerStats_.defense); });
    addStatsRow("Move Speed", [this]() { return floatToPrettyString(playerStats_.moveSpeed); });
    addStatsRow("Speed of Attack", [this]() { return floatToPrettyString(playerStats_.speedOfAttack); });
    addStatsRow("Attack Range", [this]() { return floatToPrettyString(playerStats_.attackRange); });
    addStatsRow("Dash", [this]() { return std::string(playerStats_.hasDash ? "Yes" : "No"); });

    return statsCard;
}

view::Card &ProgressionStore::createStoreItemsCard()
{
    view::Card &itemsCard = cards_.emplace_back(view::Card());
    itemsCard.rect = geometry::Rectangle<float>{.position = {itemsX, contentY}, .size = {itemsW, contentH}};
    itemsCard.backgroundColor = itemsCardColor;

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

    std::optional<PersistedStore> persistedStoreOpt = controller::PersistenceManager::getStore();
    bool isStorePersisted = persistedStoreOpt.has_value();
    PersistedStore persistedStore = persistedStoreOpt.value_or(PersistedStore{});

    for (const config::StoreItemConfig &storeItemConfig : config_.storeConfig.items) {
        if (storeItemConfig.typeToConfig.empty()) {
            logger::log(logger::LogLevel::ERROR,
                        std::format("Store item {} has no type configurations.", storeItemConfig.name));
            continue;
        }

        StoreItemType selectedType;

        if (isStorePersisted) {
            auto nameToTypeIt = persistedStore.nameToSelectedType.find(storeItemConfig.name);
            if (nameToTypeIt == persistedStore.nameToSelectedType.end()) {
                logger::log(
                    logger::LogLevel::ERROR,
                    std::format("Store item {} has no selected type in persisted store.", storeItemConfig.name));
                continue;
            }
            selectedType = nameToTypeIt->second;
        } else {
            selectedType = selectRandomStoreItemType(storeItemConfig);
            persistedStore.nameToSelectedType.emplace(storeItemConfig.name, selectedType);
        }

        auto typeToConfigIt = storeItemConfig.typeToConfig.find(selectedType);
        if (typeToConfigIt == storeItemConfig.typeToConfig.end()) {
            logger::log(logger::LogLevel::ERROR, std::format("Store item {} has no configuration for selected type {}.",
                                                             storeItemConfig.name, toString(selectedType)));
            continue;
        }
        const config::StoreItemTypeConfig &typeConfig = typeToConfigIt->second;

        const int row = visibleItemIndex / itemColumns;
        const int col = visibleItemIndex % itemColumns;

        const float itemX = itemGridStartX + col * (itemSize + itemGapX);
        const float itemY = itemGridStartY + row * (itemSize + itemGapY);

        view::Card &itemCard = cards_.emplace_back(view::Card());
        itemCard.rect = geometry::Rectangle<float>{.position = {itemX, itemY}, .size = {itemSize, itemSize}};
        itemCard.backgroundColor = storeItemCardColor;

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
            prevSelectedStoreItemIndex_ = itemIndex;
            selectedStoreItemIndex_ = itemIndex;
        }

        ++visibleItemIndex;
    }

    if (!isStorePersisted) {
        controller::PersistenceManager::saveStore(persistedStore);
    }

    return itemsCard;
}

StoreItemType ProgressionStore::selectRandomStoreItemType(const config::StoreItemConfig &storeItemConfig)
{
    std::vector<StoreItemType> availableTypes;
    std::vector<float> baseWeights;

    for (const auto &[type, typeConfig] : storeItemConfig.typeToConfig) {
        availableTypes.push_back(type);
        baseWeights.push_back(config_.storeConfig.typeToRandomWeight.at(type));
    }

    const std::vector<float> probabilities = getWaveAdjustedProbabilities(baseWeights, game_.getWaveNumber());

    std::discrete_distribution<std::size_t> distribution(probabilities.begin(), probabilities.end());
    return availableTypes[distribution(randomEngine_)];
}

view::Card &ProgressionStore::createSelectedItemDetailsCard()
{
    view::Card &detailsCard = cards_.emplace_back(view::Card());
    detailsCard.rect = geometry::Rectangle<float>{.position = {detailsX, contentY}, .size = {detailsW, contentH}};
    detailsCard.backgroundColor = detailsCardColor;

    auto addDynamicText = [this, &detailsCard](geometry::Vec2<float> position, unsigned int size,
                                               std::function<std::string()> getText) {
        view::Text &text = texts_.emplace_back(view::Text());
        text.position = position;
        text.size = size;

        dynamicTexts_.push_back(DynamicText{
            .textView = text,
            .getText = getText,
        });

        detailsCard.elements.push_back(text);
    };

    addDynamicText({detailsCard.rect.getCenter().x, contentY + 55.0f}, 28, [this]() {
        const StoreItem *item = getSelectedStoreItem();
        return item != nullptr ? item->itemConfig.name : "No item selected";
    });

    view::Card &selectedItemCard = cards_.emplace_back(view::Card());
    selectedItemCard.rect = geometry::Rectangle<float>{
        .position = {detailsCard.rect.getCenter().x - 48.0f, contentY + 110.0f}, .size = {96.0f, 96.0f}};
    selectedItemCard.backgroundColor = storeItemCardColor;

    selectedItemIcon_.rect = geometry::Rectangle<float>{
        .position = {selectedItemCard.rect.position.x + 8.0f, selectedItemCard.rect.position.y + 8.0f},
        .size = {80.0f, 80.0f}};

    selectedItemCard.elements.push_back(selectedItemIcon_);
    detailsCard.elements.push_back(selectedItemCard);

    addDynamicText({detailsCard.rect.getCenter().x, contentY + 235.0f}, 22, [this]() {
        const StoreItem *item = getSelectedStoreItem();
        return item != nullptr ? toString(item->type) : "";
    });

    addDynamicText({detailsCard.rect.getCenter().x, contentY + 290.0f}, 20, [this]() {
        const StoreItem *item = getSelectedStoreItem();
        return item != nullptr ? item->itemConfig.description : "";
    });

    addDynamicText({detailsCard.rect.getCenter().x, contentY + 365.0f}, 20, [this]() {
        const StoreItem *item = getSelectedStoreItem();
        return item != nullptr ? getStatChangesText(item->typeConfig.statChanges) : "";
    });

    addDynamicText({detailsCard.rect.getCenter().x, contentY + 455.0f}, 22, [this]() {
        const StoreItem *item = getSelectedStoreItem();
        return item != nullptr ? "Cost: " + std::to_string(item->typeConfig.cost) + " Gold" : "";
    });

    auto buyButtonRect = geometry::Rectangle<float>{.position = {detailsX + 36.0f, contentY + contentH - 92.0f},
                                                    .size = {detailsW - 72.0f, 64.0f}};

    buyButton_ = &createButton(buyButtonRect, ButtonTypeId::Buy, buyButtonText);
    detailsCard.elements.push_back(*buyButton_);

    return detailsCard;
}

const StoreItem *ProgressionStore::getSelectedStoreItem() const
{
    if (!selectedStoreItemIndex_.has_value()) {
        return nullptr;
    }

    const std::size_t index = selectedStoreItemIndex_.value();

    if (index >= storeItems_.size()) {
        return nullptr;
    }

    return &storeItems_[index];
}

std::string ProgressionStore::getStatChangesText(const PlayerStats &statChanges) const
{
    std::vector<std::string> changes;

    auto addFloatChange = [&changes](const std::string &label, float value) {
        if (value != 0.0f) {
            const std::string sign = value > 0.0f ? "+" : "";
            changes.push_back(label + ": " + sign + floatToPrettyString(value));
        }
    };

    auto addIntChange = [&changes](const std::string &label, int value) {
        if (value != 0) {
            const std::string sign = value > 0 ? "+" : "";
            changes.push_back(label + ": " + sign + std::to_string(value));
        }
    };

    addFloatChange("Max Health", statChanges.maxHealth);
    addFloatChange("Attack", statChanges.attackPower);
    addFloatChange("Attack Speed", statChanges.attackSpeed);
    addFloatChange("Special Attack Speed", statChanges.specialAttackSpeed);
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

bool ProgressionStore::updateButtonSelection(const controller::InputState &input)
{
    prevSelectedButtonIndex_ = selectedButtonIndex_;

    const bool isMouseSelectionActive = input.mouseMoved || input.mouseLeftPressed;
    const std::optional<std::size_t> hoveredButtonIndex = controller::MouseUtil::getHoveredButtonId(input, buttons_);

    if (isMouseSelectionActive && hoveredButtonIndex.has_value()) {
        selectedButtonIndex_ = hoveredButtonIndex.value();
    }

    const bool isButtonHovered = hoveredButtonIndex.has_value();
    const bool buttonPressed = input.confirmPressed || (input.mouseLeftPressed && isButtonHovered);

    for (std::size_t i = 0; i < buttons_.size(); ++i) {
        buttons_[i].isSelected = i == selectedButtonIndex_;
    }

    const view::Button &selectedButton = buttons_[selectedButtonIndex_];
    if (selectedButton.id == ButtonTypeId::Buy && !canBuySelectedStoreItem()) {
        return false;
    }

    return buttonPressed;
}

void ProgressionStore::updateStoreItemSelection(const controller::InputState &input)
{
    prevHoveredStoreItemIndex_ = hoveredStoreItemIndex_;
    prevSelectedStoreItemIndex_ = selectedStoreItemIndex_;

    if (!input.mouseMoved && !input.mouseLeftPressed) {
        return;
    }

    hoveredStoreItemIndex_ = getHoveredStoreItemIndex(input);

    if (input.mouseLeftPressed && hoveredStoreItemIndex_.has_value()) {
        selectedStoreItemIndex_ = hoveredStoreItemIndex_;
        updateSelectedItemIcon();
    }
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

void ProgressionStore::updateSelectedItemIcon()
{
    const StoreItem *item = getSelectedStoreItem();
    selectedItemIcon_.imagePath = item != nullptr ? item->typeConfig.icon.path : "";
}

void ProgressionStore::updateDynamicTexts()
{
    for (DynamicText &dynamicText : dynamicTexts_) {
        dynamicText.textView.text = dynamicText.getText();
    }
}

void ProgressionStore::updateStoreItemLayouts()
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
            item.card.backgroundColor = selectedStoreItemCardColor;
        } else if (isHovered) {
            item.card.backgroundColor = hoveredStoreItemCardColor;
        } else {
            item.card.backgroundColor = storeItemCardColor;
        }
    }
}

bool ProgressionStore::canBuySelectedStoreItem() const
{
    const StoreItem *item = getSelectedStoreItem();
    return item != nullptr && playerStats_.currency >= item->typeConfig.cost;
}

bool ProgressionStore::buySelectedStoreItem()
{
    const StoreItem *item = getSelectedStoreItem();

    if (item == nullptr) {
        return false;
    }

    if (!canBuySelectedStoreItem()) {
        logger::log(logger::LogLevel::INFO,
                    std::format("Cannot buy store item '{}'. Cost: {}, Currency: {}.", item->itemConfig.name,
                                item->typeConfig.cost, playerStats_.currency));

        return false;
    }

    playerStats_.currency -= item->typeConfig.cost;
    applyStatChanges(item->typeConfig.statChanges);
    game_.save();
    buyButtonPressed_ = true;

    logger::log(logger::LogLevel::INFO, std::format("Bought store item '{}' ({}) for {} gold.", item->itemConfig.name,
                                                    toString(item->type), item->typeConfig.cost));

    return true;
}

void ProgressionStore::applyStatChanges(const PlayerStats &statChanges)
{
    playerStats_.maxHealth += statChanges.maxHealth;
    playerStats_.health += statChanges.health;
    playerStats_.attackPower += statChanges.attackPower;
    playerStats_.attackSpeed += statChanges.attackSpeed;
    playerStats_.specialAttackSpeed += statChanges.specialAttackSpeed;
    playerStats_.defense += statChanges.defense;
    playerStats_.moveSpeed += statChanges.moveSpeed;
    playerStats_.speedOfAttack += statChanges.speedOfAttack;
    playerStats_.attackRange += statChanges.attackRange;

    if (statChanges.hasDash) {
        playerStats_.hasDash = true;
    }

    playerStats_.enemiesPierced += statChanges.enemiesPierced;
}

void ProgressionStore::updateBuyButtonState()
{
    if (buyButton_ == nullptr) {
        return;
    }

    if (canBuySelectedStoreItem()) {
        buyButton_->backgroundColor = buyButtonColor;
        buyButton_->selectedColor = buyButtonSelectedColor;
        buyButton_->text.color = enabledBuyButtonTextColor;
    } else {
        buyButton_->backgroundColor = disabledBuyButtonColor;
        buyButton_->selectedColor = disabledBuyButtonSelectedColor;
        buyButton_->text.color = disabledBuyButtonTextColor;
    }
}

} // namespace game