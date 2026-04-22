#include "game/game.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"
#include <controller/view/text.hpp>
#include <iostream>

namespace game {

Game::Game(controller::View &view) : view_(view)
{
    std::cout << "Game constructed" << std::endl;
    initPlayer();
}

void Game::initPlayer()
{
    Entity player = registry_.createEntity();
    registry_.addComponent<PlayerTag>(player, {});
    registry_.addComponent<Position>(player, {100.0f, 100.0f});
    registry_.addComponent<Velocity>(player, {0.0f, 0.0f});
}

Game::~Game()
{
    std::cout << "Game destructed" << std::endl;
}

GameDebugSession &Game::getDebugSession()
{
    return debugSession_;
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
    if (debugSession_.isStageWaveReloadRequested) {
        debugSession_.isStageWaveReloadRequested = false;
        std::cout << "Reloading stage " << debug.gameSettings.stage << ", wave " << debug.gameSettings.wave
                  << std::endl;
        // TODO: Implement actual stage/wave reloading logic
    }

    // Handle player destruction request
    if (debugSession_.isPlayerDestructionRequested) {
        debugSession_.isPlayerDestructionRequested = false;
        std::cout << "Destroying player entity!" << std::endl;
        for (Entity player : registry_.view<PlayerTag>()) {
            registry_.destroyEntity(player);
        }
    }
}

void Game::updateSystems(const controller::InputState &input, controller::DebugContext &debug, float dt)
{
    if (debug.active && !debugSession_.isSystemUpdateActive) {
        return;
    }

    inputSystem_.update(registry_, input);
    movementSystem_.update(registry_, dt);
}

bool Game::isGameOver()
{
    return registry_.view<PlayerTag>().empty();
}

const controller::View &Game::getView()
{
    return view_;
}

} // namespace game