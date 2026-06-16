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
    std::unordered_map<view::FontType, sf::Font> fontTypeToSfFont_;
    std::map<std::string, sf::Texture> textureCache_;
    const sf::Font &toSfFont(const view::FontType font);
    sf::Texture &getTexture(const std::string &imagePath);
    void renderElement(sf::RenderWindow &window, const view::Card &card);
    void renderElement(sf::RenderWindow &window, const view::Button &button);
    void renderElement(sf::RenderWindow &window, const view::Text &text);
    void renderElement(sf::RenderWindow &window, const view::Sprite &sprite);
    void renderElement(sf::RenderWindow &window, const view::Rectangle &rectangle);
    void renderDebugLocationTable(sf::RenderWindow &window);

  public:
    Renderer();
    ~Renderer();

    void renderViewElement(sf::RenderWindow &window, const view::ViewElement &element);
    void renderViewElements(sf::RenderWindow &window, const std::vector<view::ViewElement> &elements);
    sf::Color toSfColor(const view::Color &color);
    void renderDebugContext(sf::RenderWindow &window);
};

} // namespace ui