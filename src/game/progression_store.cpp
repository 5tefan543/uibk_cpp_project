#pragma once

#include "game/progression_store.hpp"
#include "controller/input/mouse_util.hpp"

namespace game {

game::ProgressionStore::ProgressionStore(const Game &game) : game(game) {}

void game::ProgressionStore::initView(view::View &view)
{
    view::Card &backgroundCard = cards_.emplace_back(view::Card());
    backgroundCard.rect = view::grid;

    view::Card &storeCard = cards_.emplace_back(view::Card());
    storeCard.backgroundColor = {50, 50, 50};
    const auto storeCardCenter = storeCard.rect.getCenter();

    view::Text &title = texts_.emplace_back(view::Text());
    title.position.y = (storeCard.rect.position.y + storeCard.rect.size.y / 10);
    title.text = std::string("Store Menu");

    view::Button &startGameButton = buttons_.emplace_back(view::Button());
    startGameButton.rect.centerizeY(storeCardCenter.y - startGameButton.rect.size.y);
    startGameButton.text.position.y = startGameButton.rect.getCenter().y;
    startGameButton.text.text = std::string("Continue Game");

    view::Button &quitButton = buttons_.emplace_back(view::Button());
    quitButton.rect.centerizeY(storeCardCenter.y + quitButton.rect.size.y);
    quitButton.text.position.y = quitButton.rect.getCenter().y;
    quitButton.text.text = std::string("Quit Game");

    storeCard.elements.push_back(title);
    storeCard.elements.push_back(startGameButton);
    storeCard.elements.push_back(quitButton);
    backgroundCard.elements.push_back(storeCard);
    view.nodes.push_back({view::ViewMode::FixedToScreen, backgroundCard});

    buttons_[selectedButtonId_].isSelected = true;
}

controller::StateTransitionAction ProgressionStore::update(const controller::InputState &input)
{
    prevSelectedButtonId_ = selectedButtonId_;

    controller::StateTransitionAction stateTransitionAction = controller::StateTransitionAction::None;
    bool isMouseSelectionActive = input.mouseMoved || input.mouseLeftPressed;

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

    if (buttonPressed) {
        switch (selectedButtonId_) {
        case 0:
            stateTransitionAction = controller::StateTransitionAction::Pop;
            break;
        case 1:
            stateTransitionAction = controller::StateTransitionAction::ReplaceAllStatesWithExit;
            break;
        }
    }

    buttons_[prevSelectedButtonId_].isSelected = false;
    buttons_[selectedButtonId_].isSelected = true;

    return stateTransitionAction;
}

bool ProgressionStore::selectedButtonChanged()
{
    return selectedButtonId_ != prevSelectedButtonId_;
}

void ProgressionStore::updateView(view::View &view)
{
    // TODO: do we need that?
}

} // namespace game