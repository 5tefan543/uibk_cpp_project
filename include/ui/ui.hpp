#pragma once
#include "ui/input_handler.hpp"
#include "ui/renderer.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

namespace ui {

class UI {
  private:
	sf::RenderWindow window;
	InputHandler input_handler;
	Renderer renderer;

	void initWindow();

  public:
	UI();
	~UI();

	bool isOpen() const;
	controller::InputState pollInput();
	void render();
};

} // namespace ui

int foo2();