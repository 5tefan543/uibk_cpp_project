#include "game/game.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"
#include <controller/view/text.hpp>
#include <iostream>

namespace game {

Game::Game()
{
    std::cout << "Game constructed" << std::endl;
    initPlayer();
}

Game::~Game()
{
    std::cout << "Game destructed" << std::endl;
}

void Game::initPlayer()
{
    Entity player = registry.createEntity();
    registry.addComponent<PlayerTag>(player, {});
    registry.addComponent<Position>(player, {100.0f, 100.0f});
    registry.addComponent<Velocity>(player, {0.0f, 0.0f});
}

controller::StateTransitionAction Game::update(const controller::InputState &input, controller::DebugState &debug,
                                               float dt)
{
    inputSystem.update(registry, input);
    movementSystem.update(registry, dt);

    controller::StateTransitionAction action = controller::StateTransitionAction::None;

    if (input.cancelPressed) {
        action = controller::StateTransitionAction::PushPauseMenu;
    } else if (input.mouseLeftPressed) {
        action = controller::StateTransitionAction::PushProgressionStore;
    } else if (input.confirmPressed) {
        action = controller::StateTransitionAction::ReplaceCurrentWithGameOverMenu;
    } else if (input.mouseRightPressed) {
        if (registry.view<PlayerTag>().empty()) {
            initPlayer();
        } else {
            for (Entity entity : registry.view<PlayerTag>()) {
                registry.destroyEntity(entity);
            }

            // for (Entity entity : registry.view<Position>()) {
            //     registry.removeComponent<Position>(entity);
            // }
        }
    } else if (input.mouseMiddlePressed) {
        initPlayer();
    }

    handleDebugState(debug, action);

    return action;
}

void Game::handleDebugState(controller::DebugState &debug, controller::StateTransitionAction &action)
{
    if (debug.active) {

        // reset debug info on game over
        if (action == controller::StateTransitionAction::ReplaceCurrentWithGameOverMenu) {
            debug.game.registry = nullptr;
            debug.game.selectedEntity.reset();
            return;
        }

        // ecs management
        debug.game.registry = &registry;

        // Handle stage/wave reload request
        if (debug.game.isStageWaveReloadRequested) {
            debug.game.isStageWaveReloadRequested = false;
            std::cout << "Reloading stage " << debug.game.stage << ", wave " << debug.game.wave << std::endl;
            // TODO: Implement actual stage/wave reloading logic
        }
    }
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