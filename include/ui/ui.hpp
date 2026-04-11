#pragma once

#include "controller/view/font.hpp"
#include "controller/view/view.hpp"
#include "ui/input_handler.hpp"
#include "ui/renderer.hpp"

#include <SFML/Graphics/Font.hpp>
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
    void render(const controller::View &view);
    sf::Font getFont(const controller::Font font);
};

} // namespace ui