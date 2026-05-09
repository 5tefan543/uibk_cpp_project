#pragma once

#include "view/view.hpp"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <map>
#include <string>

namespace ui {

class Renderer {
  private:
    std::vector<sf::Font> fonts_;
    std::map<std::string, sf::Texture> textureCache_;
    float cameraX_ = 0.0f;
    float cameraY_ = 0.0f;

  public:
    Renderer();
    ~Renderer();

    sf::Color toSfColor(const view::Color &color);
    const sf::Font &toSfFont(const view::Font font);
    sf::Text toSfText(const view::Text text);

    void renderView(sf::RenderWindow &window, const view::View &view);
    void renderItems(sf::RenderWindow &window, const std::vector<view::ViewItem> &items);
    void renderItem(sf::RenderWindow &window, const view::Card &card);
    void renderItem(sf::RenderWindow &window, const view::Button &button);
    void renderItem(sf::RenderWindow &window, const view::Text &text);
    void renderItem(sf::RenderWindow &window, const view::Sprite &sprite);
};

} // namespace ui