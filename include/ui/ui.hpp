#pragma once

#include "controller/view/font.hpp"
#include "controller/view/view.hpp"
#include "ui/debug_ui.hpp"
#include "ui/input_handler.hpp"
#include "ui/renderer.hpp"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace ui {

class UI {
  private:
    sf::RenderWindow window;
    InputHandler inputHandler;
    Renderer renderer;
    sf::Clock imguiClock;
    DebugUI debugUI;

    void initWindow();
    void initDebugWindow(sf::RenderWindow &window);

  public:
    UI();
    ~UI();

    bool isOpen() const;
    controller::InputState pollInput();
    void render(const controller::View &view, controller::DebugState &debugState);
};

} // namespace ui