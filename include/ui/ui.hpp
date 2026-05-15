#pragma once

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
    sf::Clock imguiClock_;
    DebugUI debugUI_;
    controller::InputState inputState_;
    float fps_ = 0.0f;

    void initSfmlWindow();
    void initImGuiSfml();
    void setSfmlView(float cameraX, float cameraY);
    sf::FloatRect getLetterboxViewport() const;
    void renderView(sf::RenderWindow &window, const view::View &view);

  public:
    UI();
    ~UI();

    bool isOpen() const;
    const controller::InputState &pollInput();
    void render(const view::View &view);
};

} // namespace ui