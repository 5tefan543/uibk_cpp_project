#include "ui/renderer.hpp"
#include "controller/view/grid.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>

namespace ui {

Renderer::Renderer()
{
    // Load all fonts from disk once upon instantiation
    fonts_ = std::vector<sf::Font>({sf::Font("assets/font/BigBlueTerm_Nerd_Font/BigBlueTerm437NerdFont-Regular.ttf")});
    std::cout << "Renderer constructed" << std::endl;
}

Renderer::~Renderer()
{
    std::cout << "Renderer destructed" << std::endl;
}

sf::Color Renderer::toSfColor(const controller::Color &color)
{
    return sf::Color(color.red, color.green, color.blue);
}

const sf::Font &Renderer::toSfFont(const controller::Font font)
{
    return fonts_.at(font);
}

void Renderer::renderView(sf::RenderWindow &window, const controller::View &view, const bool windowResized)
{
    using controller::gridHeight;
    using controller::gridWidth;

    // Store camera data
    cameraX_ = view.cameraX;
    cameraY_ = view.cameraY;

    if (windowResized) {
        auto winSize = window.getSize();

        const float widthScaled = gridWidth * ((float)winSize.x / gridWidth);
        const float heightScaled = gridHeight * ((float)winSize.y / gridHeight);

        const float widthOffset = (gridWidth - winSize.x) / 2;
        const float heightOffset = (gridHeight - winSize.y) / 2;

        // Always based on original view parameters and not last ones (e.g. window.getView()) which would throw of all
        // calculations
        auto v = sf::View(sf::FloatRect({widthOffset, heightOffset}, {widthScaled, heightScaled}));

        // large content < 0 > small content (zoom <~> camera distance)
        v.zoom(std::max((float)gridWidth / winSize.x, (float)gridHeight / winSize.y));

        // setView() has to be called upon every change (render-target makes a copy of the view) src: "Using a
        // View" https://www.sfml-dev.org/tutorials/3.0/graphics/view/#defining-how-the-view-is-viewed
        window.setView(v);
    }
    renderItems(window, view.items);
}

void Renderer::renderItems(sf::RenderWindow &window, const std::vector<controller::ViewItem> &items)
{
    for (const controller::ViewItem &item : items) {
        std::visit([this, &window](const auto &item) { renderItem(window, item); }, item);
    }
}

void Renderer::renderItem(sf::RenderWindow &window, const controller::Card &card)
{
    // Render card first
    sf::RectangleShape rect;
    rect.setSize({card.width, card.height});
    rect.setPosition({card.gridX, card.gridY});
    rect.setFillColor(toSfColor(card.backgroundColor));
    window.draw(rect);

    // Render items on the card
    renderItems(window, card.items);
}

void Renderer::renderItem(sf::RenderWindow &window, const controller::Button &button)
{
    sf::RectangleShape rect;
    rect.setSize({button.width, button.height});
    rect.setPosition({button.gridX, button.gridY});
    rect.setFillColor(button.isSelected ? toSfColor(button.selectedColor) : toSfColor(button.backgroundColor));
    window.draw(rect);

    renderItem(window, button.text);
}

void Renderer::renderItem(sf::RenderWindow &window, const controller::Text &text)
{
    sf::Text t(toSfFont(text.font), text.text, text.size);
    t.setPosition(sf::Vector2f(text.gridX, text.gridY));
    t.setFillColor(toSfColor(text.color));

    sf::Vector2<float> pos = t.getLocalBounds().getCenter();
    pos.x += text.originOffsetX;
    pos.y += text.originOffsetY;
    t.setOrigin(pos);

    window.draw(t);
}

void Renderer::renderItem(sf::RenderWindow &window, const controller::Sprite &sprite)
{
    // Load or get texture from cache
    if (textureCache_.find(sprite.imagePath) == textureCache_.end()) {
        sf::Texture texture;
        if (!texture.loadFromFile(sprite.imagePath)) {
            std::cerr << "Failed to load texture: " << sprite.imagePath << std::endl;
            return;
        }
        textureCache_[sprite.imagePath] = texture;
    }

    sf::Sprite sfSprite(textureCache_[sprite.imagePath]);

    // Calculate position with camera offset and scaling
    float x = sprite.x;
    float y = sprite.y;
    float scale = sprite.scale;

    // Apply camera offset only if not a map
    if (!sprite.isMap) {
        x -= cameraX_;
        y -= cameraY_;
    }

    // Apply scaling
    x *= scale;
    y *= scale;

    sfSprite.setPosition(sf::Vector2f(x, y));

    float scaledWidth = sprite.width * scale / sfSprite.getLocalBounds().size.x;
    float scaledHeight = sprite.height * scale / sfSprite.getLocalBounds().size.y;

    sfSprite.setScale(sf::Vector2f(scaledWidth, scaledHeight));

    window.draw(sfSprite);
}

} // namespace ui