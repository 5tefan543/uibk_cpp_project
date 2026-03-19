#include "ui/input_handler.hpp"
#include "controller/input_state.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>

namespace ui {

InputHandler::InputHandler()
{
	std::cout << "InputHandler constructed" << std::endl;
}

InputHandler::~InputHandler()
{
	std::cout << "InputHandler destructed" << std::endl;
}

controller::InputState InputHandler::pollInput(sf::RenderWindow &window)
{
	controller::InputState state;
	while (const std::optional event = window.pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			window.close();
		}
	}

	state.left = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
	state.right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
	state.up = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
	state.down = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
	state.mouse_left = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
	state.mouse_right = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
	state.mouse_middle = sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle);
	state.mouse_x = sf::Mouse::getPosition(window).x;
	state.mouse_y = sf::Mouse::getPosition(window).y;
	return state;
}

} // namespace ui