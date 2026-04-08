#pragma once

#include "controller/view/view.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

namespace ui {

class Renderer {
  public:
    Renderer();
    ~Renderer();

    sf::Color toSfColor(const controller::Color &color);

    void renderItems(sf::RenderWindow &window, const std::vector<controller::ViewItem> &items);
    void renderItem(sf::RenderWindow &window, const std::unique_ptr<controller::Card> &card);
    void renderItem(sf::RenderWindow &window, const controller::Button &button);
};

} // namespace ui