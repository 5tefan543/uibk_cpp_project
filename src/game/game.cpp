#include "game/game.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"
#include <controller/view/text.hpp>
#include <iostream>

namespace game {

Game::Game()
{
    std::cout << "Game constructed" << std::endl;
    initEntities();
}

Game::~Game()
{
    std::cout << "Game destructed" << std::endl;
}

void Game::initEntities()
{
    Entity player = registry.createEntity();
    registry.addComponent<Position>(player, {100.0f, 100.0f});
    registry.addComponent<Velocity>(player, {0.0f, 0.0f});
}

controller::StateTransitionAction Game::update(const controller::InputState &input, float dt)
{
    inputSystem.update(registry, input);
    movementSystem.update(registry, dt);

    if (input.pause) {
        return controller::StateTransitionAction::PushPauseMenu;
    } else if (input.mouseLeft) {
        return controller::StateTransitionAction::PushProgressionStore;
    } else if (input.confirm) {
        return controller::StateTransitionAction::ReplaceCurrentWithGameOverMenu;
    } else if (input.mouseRight) {
        if (registry.entities().empty()) {
            initEntities();
        } else {
            auto view = registry.view<Position, Velocity>();
            for (auto entity : view) {
                registry.destroyEntity(entity);
            }
        }
    }
    return controller::StateTransitionAction::None;
}

controller::View Game::getView()
{
    controller::View view;
    // TODO: Construct view based on game state
    controller::Text text;
    text.text = std::string("<< GAMEPLAY STATE PLACEHOLDER >>\n\n"
                            "- left-mouse-btn -> Progression Store\n"
                            "- enter          -> Game Over\n"
                            "- esc            -> Pause Menu");
    view.items.push_back(text);
    return view;
}

} // namespace game