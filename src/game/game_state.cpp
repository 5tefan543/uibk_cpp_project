#include "game/game_state.hpp"
#include <iostream>

namespace game {

GameState::GameState()
{
    std::cout << "GameState constructed" << std::endl;
}

GameState::~GameState()
{
    std::cout << "GameState destructed" << std::endl;
}

} // namespace game
