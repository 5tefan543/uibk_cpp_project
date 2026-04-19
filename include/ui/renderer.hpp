#pragma once

#include "controller/view/view.hpp"
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
    float mapWidth_ = 1920.0f;
    float mapHeight_ = 1080.0f;

  public:
    Renderer();
    ~Renderer();

    sf::Color toSfColor(const controller::Color &color);
    const sf::Font &toSfFont(const controller::Font font);
    sf::Text toSfText(const controller::Text text);

    void renderItems(sf::RenderWindow &window, const controller::View &view);
    void renderItem(sf::RenderWindow &window, const std::vector<controller::ViewItem> &items);
    void renderItem(sf::RenderWindow &window, const std::unique_ptr<controller::Card> &card);
    void renderItem(sf::RenderWindow &window, const controller::Button &button);
    void renderItem(sf::RenderWindow &window, const controller::Text &text);
    void renderItem(sf::RenderWindow &window, const controller::Sprite &sprite);
};

} // namespace ui