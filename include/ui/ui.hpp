#pragma once

#include "controller/timing.hpp"
#include "ui/debug_ui.hpp"
#include "ui/input_handler.hpp"
#include "ui/renderer.hpp"
#include "view/view.hpp"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace ui {

class UI {
  private:
    sf::RenderWindow window_;
    InputHandler inputHandler_;
    Renderer renderer_;
    DebugUI debugUI_;
    controller::InputState inputState_;

    void initSfmlWindow();
    void initImGuiSfml();
    void setSfmlView(geometry::Vec2<float> cameraPosition);
    sf::FloatRect getLetterboxViewport() const;
    void renderView(sf::RenderWindow &window, const view::View &view);

  public:
    UI();
    ~UI();

    bool isOpen() const;
    const controller::InputState &pollInput();

    void render(const view::View &view, const controller::timeDelta &dtSec);
};

} // namespace ui