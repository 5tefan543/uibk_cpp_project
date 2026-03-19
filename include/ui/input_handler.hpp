#pragma once
#include "controller/input_state.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

namespace ui {

class InputHandler {
  public:
	InputHandler();
	~InputHandler();

	controller::InputState pollInput(sf::RenderWindow &window);
};

} // namespace ui