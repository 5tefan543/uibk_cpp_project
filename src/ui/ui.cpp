#include "ui/ui.hpp"

#include <SFML/Window.hpp>
#include <iostream>
#include <optional>

namespace ui {

UI::UI()
{
	std::cout << "UI constructed" << std::endl;
	initWindow();
}

UI::~UI()
{
	std::cout << "UI destructed" << std::endl;
}

void UI::initWindow()
{
	window.create(sf::VideoMode({800, 600}), "My Game");
	window.setFramerateLimit(60);
}

bool UI::isOpen() const
{
	return window.isOpen();
}

controller::InputState UI::pollInput()
{
	return input_handler.pollInput(window);
}

void UI::render()
{
	window.clear(sf::Color::Black);
	renderer.render(window);
	window.display();
}

} // namespace ui