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

void Game::initPlayer()
{
    Entity player = registry.createEntity();
    registry.addComponent<PlayerTag>(player, {});
    registry.addComponent<Position>(player, {100.0f, 100.0f});
    registry.addComponent<Velocity>(player, {0.0f, 0.0f});
}

Game::~Game()
{
    std::cout << "Game destructed" << std::endl;
}

GameDebugSession &Game::getDebugSession()
{
    return debugSession;
}

bool Game::update(const controller::InputState &input, controller::DebugContext &debug, float dt)
{
    processDebugSession(debug);
    updateSystems(input, debug, dt);
    return isGameOver();
}

void Game::processDebugSession(controller::DebugContext &debug)
{
    if (!debug.active) {
        return;
    }

    // Handle stage/wave reload request
    if (debugSession.isStageWaveReloadRequested) {
        debugSession.isStageWaveReloadRequested = false;
        std::cout << "Reloading stage " << debug.gameSettings.stage << ", wave " << debug.gameSettings.wave
                  << std::endl;
        // TODO: Implement actual stage/wave reloading logic
    }

    // Handle player destruction request
    if (debugSession.isPlayerDestructionRequested) {
        debugSession.isPlayerDestructionRequested = false;
        std::cout << "Destroying player entity!" << std::endl;
        for (Entity player : registry.view<PlayerTag>()) {
            registry.destroyEntity(player);
        }
    }
}

void Game::updateSystems(const controller::InputState &input, controller::DebugContext &debug, float dt)
{
    if (debug.active && !debugSession.isSystemUpdateActive) {
        return;
    }

    inputSystem.update(registry, input);
    movementSystem.update(registry, dt);
}

bool Game::isGameOver()
{
    return registry.view<PlayerTag>().empty();
}

controller::View Game::getView()
{
    controller::View view;
    // TODO: Construct view based on game state
    return view;
}

} // namespace game