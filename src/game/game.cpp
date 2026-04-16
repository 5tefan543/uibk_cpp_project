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

controller::StateTransitionAction Game::update(const controller::InputState &input, controller::DebugContext &debug,
                                               float dt)
{
    initDebugContext(debug);
    updateSystems(input, debug, dt);
    controller::StateTransitionAction action = determineStateAction(input, debug);
    cleanUpDebugContext(action, debug);
    return action;
}

void Game::initDebugContext(controller::DebugContext &debug)
{
    debug.gameSession = &debugSession; // TODO: set outside or inside active check???

    if (!debug.active) {
        return;
    }

    // Handle stage/wave reload request
    if (debug.gameSession->isStageWaveReloadRequested) {
        debug.gameSession->isStageWaveReloadRequested = false;
        std::cout << "Reloading stage " << debug.gameSettings.stage << ", wave " << debug.gameSettings.wave
                  << std::endl;
        // TODO: Implement actual stage/wave reloading logic
    }

    // Handle player destruction request
    if (debug.gameSession->isPlayerDestructionRequested) {
        debug.gameSession->isPlayerDestructionRequested = false;
        std::cout << "Destroying player entity!" << std::endl;
        for (Entity player : registry.view<PlayerTag>()) {
            registry.destroyEntity(player);
        }
    }
}

void Game::updateSystems(const controller::InputState &input, controller::DebugContext &debug, float dt)
{
    if (debug.active && !debug.gameSession->isSystemUpdateActive) {
        return;
    }

    inputSystem.update(registry, input);
    movementSystem.update(registry, dt);
}

controller::StateTransitionAction Game::determineStateAction(const controller::InputState &input,
                                                             controller::DebugContext &debug)
{
    if (input.cancelPressed) {
        return controller::StateTransitionAction::PushPauseMenu;
    }

    if (registry.view<PlayerTag>().empty()) {
        return controller::StateTransitionAction::ReplaceCurrentWithGameOverMenu;
    }

    if (debug.active && debug.gameSession->isPushStoreRequested) {
        debug.gameSession->isPushStoreRequested = false;
        return controller::StateTransitionAction::PushProgressionStore;
    }

    return controller::StateTransitionAction::None;
}

void Game::cleanUpDebugContext(controller::StateTransitionAction action, controller::DebugContext &debug)
{
    if (action == controller::StateTransitionAction::ReplaceCurrentWithGameOverMenu) {
        debug.gameSession = nullptr;
    }
}

controller::View Game::getView()
{
    controller::View view;
    // TODO: Construct view based on game state
    return view;
}

} // namespace game