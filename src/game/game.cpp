#include "game/game.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"
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
    Entity player = registry.create();
    registry.emplace<Position>(player, 100.0f, 100.0f);
    registry.emplace<Velocity>(player, 0.0f, 0.0f);
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
    }
    return controller::StateTransitionAction::None;
}

controller::View Game::getView()
{
    controller::View view;
    // TODO: Construct view based on game state
    return view;
}

} // namespace game