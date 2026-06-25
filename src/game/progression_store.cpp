#include "game/progression_store.hpp"
#include "controller/debug/debug_context.hpp"
#include "controller/input/mouse_util.hpp"

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
const float detailsW = contentW * 0.24f;
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

} // namespace

ProgressionStore::ProgressionStore(Game &game) : game_(game), playerStats_(game.getPlayerStats()) {}

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
        labelText.alignment = view::TextAlignment::Right;
        labelText.size = statTextSize;

        view::Text &valueText = texts_.emplace_back(view::Text());
        valueText.position = {valueX, statY};
        valueText.alignment = view::TextAlignment::Left;
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

    constexpr int itemColumns = 5;
    constexpr int itemRows = 2;

    constexpr float itemSize = 82.0f;
    constexpr float itemGapX = 42.0f;
    constexpr float itemGapY = 92.0f;

    const float totalItemGridW = itemColumns * itemSize + (itemColumns - 1) * itemGapX;
    const float itemGridStartX = itemsX + (itemsW - totalItemGridW) * 0.5f;
    const float itemGridStartY = contentY + 135.0f;

    const std::array<std::string, 10> itemLabels = {"ATK", "DEF", "AS", "BLK", "HP", "MS", "CRIT", "REG", "SPD", "DEF"};

    const std::array<int, 10> itemCosts = {20, 25, 15, 25, 20, 20, 25, 30, 25, 20};

    for (int row = 0; row < itemRows; ++row) {
        for (int col = 0; col < itemColumns; ++col) {
            const int index = row * itemColumns + col;

            const float itemX = itemGridStartX + col * (itemSize + itemGapX);
            const float itemY = itemGridStartY + row * (itemSize + itemGapY);

            view::Card &itemCard = cards_.emplace_back(view::Card());
            itemCard.rect = geometry::Rectangle<float>{.position = {itemX, itemY}, .size = {itemSize, itemSize}};
            itemCard.backgroundColor = view::Color{55, 45, 55};

            view::Text &itemText = texts_.emplace_back(view::Text());
            itemText.text = itemLabels[index];
            itemText.position = itemCard.rect.getCenter();

            view::Text &costText = texts_.emplace_back(view::Text());
            costText.text = std::to_string(itemCosts[index]) + " Gold";
            costText.position = {itemCard.rect.getCenter().x, itemY + itemSize + 32.0f};

            itemCard.elements.push_back(itemText);

            itemsCard.elements.push_back(itemCard);
            itemsCard.elements.push_back(costText);
        }
    }
    return itemsCard;
}

view::Card &ProgressionStore::createSelectedItemDetailsCard()
{
    view::Card &detailsCard = cards_.emplace_back(view::Card());
    detailsCard.rect = geometry::Rectangle<float>{.position = {detailsX, contentY}, .size = {detailsW, contentH}};
    detailsCard.backgroundColor = view::Color{28, 28, 28};

    view::Text &detailTitle = texts_.emplace_back(view::Text());
    detailTitle.text = "Attack";
    detailTitle.position = {detailsCard.rect.getCenter().x, contentY + 55.0f};

    detailsCard.elements.push_back(detailTitle);

    view::Card &selectedItemCard = cards_.emplace_back(view::Card());
    selectedItemCard.rect = geometry::Rectangle<float>{
        .position = {detailsCard.rect.getCenter().x - 48.0f, contentY + 110.0f}, .size = {96.0f, 96.0f}};
    selectedItemCard.backgroundColor = view::Color{55, 45, 55};

    view::Text &selectedItemText = texts_.emplace_back(view::Text());
    selectedItemText.text = "ATK";
    selectedItemText.position = selectedItemCard.rect.getCenter();

    selectedItemCard.elements.push_back(selectedItemText);
    detailsCard.elements.push_back(selectedItemCard);

    view::Text &upgradeChangeText = texts_.emplace_back(view::Text());
    upgradeChangeText.text = "+2 Attack";
    upgradeChangeText.position = {detailsCard.rect.getCenter().x, contentY + 260.0f};

    view::Text &upgradeDescriptionText = texts_.emplace_back(view::Text());
    upgradeDescriptionText.text = "Increases your base attack damage.";
    upgradeDescriptionText.position = {detailsCard.rect.getCenter().x, contentY + 315.0f};

    view::Text &upgradeValueText = texts_.emplace_back(view::Text());
    upgradeValueText.text = "10  ->  12";
    upgradeValueText.position = {detailsCard.rect.getCenter().x, contentY + 390.0f};

    view::Text &upgradeCostText = texts_.emplace_back(view::Text());
    upgradeCostText.text = "Cost: 20 Gold";
    upgradeCostText.position = {detailsCard.rect.getCenter().x, contentY + 445.0f};

    detailsCard.elements.push_back(upgradeChangeText);
    detailsCard.elements.push_back(upgradeDescriptionText);
    detailsCard.elements.push_back(upgradeValueText);
    detailsCard.elements.push_back(upgradeCostText);

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

} // namespace game