#pragma once
#include <SFML/Graphics/RenderWindow.hpp>

namespace ui {

class Renderer {
  public:
    Renderer();
    ~Renderer();

    void render(sf::RenderWindow &window);
};

} // namespace ui