#pragma once

#include "controller/view/view.hpp"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>

namespace ui {

class Renderer {
  private:
    std::vector<sf::Font> fonts_;

  public:
    Renderer();
    ~Renderer();

    sf::Color toSfColor(const controller::Color &color);
    const sf::Font &toSfFont(const controller::Font font);
    sf::Text toSfText(const controller::Text text);

    void renderItems(sf::RenderWindow &window, const std::vector<controller::ViewItem> &items);
    void renderItem(sf::RenderWindow &window, const controller::Card &card);
    void renderItem(sf::RenderWindow &window, const controller::Button &button);
    void renderItem(sf::RenderWindow &window, const controller::Text &text);
};

} // namespace ui