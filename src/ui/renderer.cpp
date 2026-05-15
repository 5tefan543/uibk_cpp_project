#include "ui/renderer.hpp"
#include "view/grid.hpp"
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

sf::Color Renderer::toSfColor(const view::Color &color)
{
    return sf::Color(color.red, color.green, color.blue);
}

const sf::Font &Renderer::toSfFont(const view::Font font)
{
    return fonts_.at(font);
}

void Renderer::renderViewElement(sf::RenderWindow &window, const view::ViewElement &element)
{
    std::visit([this, &window](const auto &element) { renderElement(window, element); }, element);
}

void Renderer::renderViewElements(sf::RenderWindow &window, const std::vector<view::ViewElement> &elements)
{
    for (const view::ViewElement &element : elements) {
        renderViewElement(window, element);
    }
}

void Renderer::renderElement(sf::RenderWindow &window, const view::Card &card)
{
    // Render card first
    sf::RectangleShape rect;
    rect.setSize({card.width, card.height});
    rect.setPosition({card.gridX, card.gridY});
    rect.setFillColor(toSfColor(card.backgroundColor));
    window.draw(rect);

    // Render elements on the card
    renderViewElements(window, card.elements);
}

void Renderer::renderElement(sf::RenderWindow &window, const view::Button &button)
{
    sf::RectangleShape rect;
    rect.setSize({button.width, button.height});
    rect.setPosition({button.gridX, button.gridY});
    rect.setFillColor(button.isSelected ? toSfColor(button.selectedColor) : toSfColor(button.backgroundColor));
    window.draw(rect);

    renderViewElement(window, button.text);
}

void Renderer::renderElement(sf::RenderWindow &window, const view::Text &text)
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

void Renderer::renderElement(sf::RenderWindow &window, const view::Sprite &sprite)
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

    sfSprite.setPosition({sprite.x, sprite.y});

    auto spriteSize = sfSprite.getLocalBounds().size;
    const float scaleFactorX = sprite.width / spriteSize.x;
    const float scaleFactorY = sprite.height / spriteSize.y;

    sfSprite.setScale({scaleFactorX, scaleFactorY});

    window.draw(sfSprite);

    if (sprite.isSelected) {
        sf::RectangleShape selectionBox;
        selectionBox.setPosition(sfSprite.getPosition());
        selectionBox.setSize(sfSprite.getGlobalBounds().size);
        selectionBox.setFillColor(sf::Color::Transparent);
        selectionBox.setOutlineColor(sf::Color::Yellow);
        selectionBox.setOutlineThickness(2.0f);
        window.draw(selectionBox);
    }
}

} // namespace ui