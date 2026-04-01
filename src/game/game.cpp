#include "game/game.hpp"
#include <iostream>

namespace game {

Game::Game()
{
    std::cout << "Game constructed" << std::endl;
}

Game::Game(const Game &)
{
    std::cout << "Game copy constructed" << std::endl;
}

Game::~Game()
{
    std::cout << "Game destructed" << std::endl;
}

} // namespace game