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
} // namespace

ProgressionStore::ProgressionStore(const Game &game) : game(game) {}

void ProgressionStore::initView(view::View &view)
{
    view::Card &backgroundCard = createBackgroundCard(view);

    view::Card &goldCard = createGoldCard(view);
    backgroundCard.elements.push_back(goldCard);

    view::Card &statsCard = createPlayerStatsCard(view);
    backgroundCard.elements.push_back(statsCard);

    view::Card &itemsCard = createStoreItemsCard(view);
    backgroundCard.elements.push_back(itemsCard);

    view::Card &detailsCard = createSelectedItemDetailsCard(view);
    backgroundCard.elements.push_back(detailsCard);

    auto quitButtonRect = geometry::Rectangle<float>{.position = {contentX, bottomY}, .size = {230.0f, bottomButtonH}};
    view::Button &quitButton = createButton(view, quitButtonRect, "QUIT", ButtonId::Quit);
    backgroundCard.elements.push_back(quitButton);

    auto mainMenuButtonRect =
        geometry::Rectangle<float>{.position = {contentX + 260.0f, bottomY}, .size = {330.0f, bottomButtonH}};
    view::Button &mainMenuButton = createButton(view, mainMenuButtonRect, "MAIN MENU", ButtonId::MainMenu);
    backgroundCard.elements.push_back(mainMenuButton);

    auto nextStageButtonRect = geometry::Rectangle<float>{
        .position = {screenX + screenW - outerPadding - 420.0f, bottomY}, .size = {420.0f, bottomButtonH}};
    view::Button &nextStageButton = createButton(view, nextStageButtonRect, "NEXT STAGE", ButtonId::NextStage);
    backgroundCard.elements.push_back(nextStageButton);

    view.nodes.push_back({view::ViewMode::FixedToScreen, backgroundCard});
    updateButtonSelection();
}

view::Card &ProgressionStore::createBackgroundCard(view::View &view)
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

view::Card &ProgressionStore::createGoldCard(view::View &view)
{
    // gold card top right corner
    view::Card &goldCard = cards_.emplace_back(view::Card());
    goldCard.rect = geometry::Rectangle<float>{.position = {screenX + screenW - outerPadding - 300.0f, screenY + 24.0f},
                                               .size = {300.0f, 68.0f}};
    goldCard.backgroundColor = view::Color{25, 25, 25};

    view::Text &goldText = texts_.emplace_back(view::Text());
    goldText.text = "Gold  100";
    goldText.position = goldCard.rect.getCenter();
    goldCard.elements.push_back(goldText);

    return goldCard;
}

view::Card &ProgressionStore::createPlayerStatsCard(view::View &view)
{
    view::Card &statsCard = cards_.emplace_back(view::Card());
    statsCard.rect = geometry::Rectangle<float>{.position = {statsX, contentY}, .size = {statsW, contentH}};
    statsCard.backgroundColor = view::Color{28, 28, 28};

    view::Text &statsTitle = texts_.emplace_back(view::Text());
    statsTitle.text = "PLAYER STATS";
    statsTitle.position = {statsCard.rect.getCenter().x, contentY + 45.0f};

    statsCard.elements.push_back(statsTitle);

    const std::array<std::string, 8> statLines = {"Attack        10",  "Defense        5", "Attack Speed  1.2",
                                                  "Max Health   100",  "Move Speed     7", "Crit Chance    5%",
                                                  "Block Chance  10%", "Life Regen    1/s"};

    float statY = contentY + 105.0f;

    for (const std::string &line : statLines) {
        view::Text &statText = texts_.emplace_back(view::Text());
        statText.text = line;
        statText.position = {statsX + statsW * 0.5f, statY};

        statsCard.elements.push_back(statText);
        statY += 50.0f;
    }
    return statsCard;
}

view::Card &ProgressionStore::createStoreItemsCard(view::View &view)
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

view::Card &ProgressionStore::createSelectedItemDetailsCard(view::View &view)
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

    view::Button &buyButton = buttons_.emplace_back(view::Button());
    buyButton.rect = geometry::Rectangle<float>{.position = {detailsX + 36.0f, contentY + contentH - 92.0f},
                                                .size = {detailsW - 72.0f, 64.0f}};
    buyButton.text.text = "BUY";
    buyButton.text.position = buyButton.rect.getCenter();
    buyButton.isSelected = selectedButtonId_ == ButtonId::Buy;

    detailsCard.elements.push_back(buyButton);
    return detailsCard;
}

view::Button &ProgressionStore::createButton(view::View &view, const geometry::Rectangle<float> &rect,
                                             const std::string &text, ButtonId buttonId)
{
    view::Button &button = buttons_.emplace_back(view::Button());
    button.rect = rect;
    button.text.text = text;
    button.text.position = button.rect.getCenter();
    button.isSelected = selectedButtonId_ == buttonId;
    return button;
}

controller::StateTransitionAction ProgressionStore::update(const controller::InputState &input)
{
    prevSelectedButtonId_ = selectedButtonId_;

    controller::StateTransitionAction stateTransitionAction = controller::StateTransitionAction::None;

    const bool isMouseSelectionActive = input.mouseMoved || input.mouseLeftPressed;

    const std::optional<std::size_t> hoveredButtonId = controller::MouseUtil::getHoveredButtonId(input, buttons_);

    if (isMouseSelectionActive && hoveredButtonId.has_value()) {
        selectedButtonId_ = hoveredButtonId.value();
    }

    const bool isButtonHovered = hoveredButtonId.has_value();
    const bool buttonPressed = input.confirmPressed || (input.mouseLeftPressed && isButtonHovered);

    if (input.downPressed) {
        selectedButtonId_ = (selectedButtonId_ + 1) % buttons_.size();
    }

    if (input.upPressed) {
        selectedButtonId_ = (selectedButtonId_ + buttons_.size() - 1) % buttons_.size();
    }

    controller::DebugContext &debug = controller::DebugContext::get();

    if (buttonPressed) {
        switch (selectedButtonId_) {
        case ButtonId::Buy:
            // TODO: buy selected upgrade
            break;

        case ButtonId::Quit:
            stateTransitionAction = controller::StateTransitionAction::ReplaceAllStatesWithExit;
            debug.gameSession = nullptr; // Reset the game session when quitting
            break;

        case ButtonId::MainMenu:
            stateTransitionAction = controller::StateTransitionAction::ReplaceAllStatesWithMainMenu;
            debug.gameSession = nullptr; // Reset the game session when returning to main menu
            break;

        case ButtonId::NextStage:
            stateTransitionAction = controller::StateTransitionAction::Pop;
            break;

        default:
            break;
        }
    }

    updateButtonSelection();

    return stateTransitionAction;
}

void ProgressionStore::updateButtonSelection()
{
    for (std::size_t i = 0; i < buttons_.size(); ++i) {
        buttons_[i].isSelected = i == selectedButtonId_;
    }
}

void ProgressionStore::updateView(view::View &view)
{
    // Not needed for now because the view references the objects owned by ProgressionStore.
}

bool ProgressionStore::selectedButtonChanged()
{
    return selectedButtonId_ != prevSelectedButtonId_;
}

} // namespace game