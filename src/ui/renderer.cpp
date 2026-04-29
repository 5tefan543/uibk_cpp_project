#include "ui/renderer.hpp"
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

sf::Text Renderer::toSfText(const controller::Text text)
{
    sf::Text t(toSfFont(text.font), text.text, text.size);
    t.setPosition(sf::Vector2f(text.centerOffsetX, text.centerOffsetX));
    return t;
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
    rect.setFillColor(toSfColor(card.backgroundColor));

    auto centerX = window.getSize().x / 2.0f + card.centerOffsetX - card.width / 2.0f;
    auto centerY = window.getSize().y / 2.0f + card.centerOffsetY - card.height / 2.0f;
    rect.setPosition({centerX, centerY});

    window.draw(rect);

    // Render items on the card
    renderItems(window, card.items);
}

void Renderer::renderItem(sf::RenderWindow &window, const controller::Button &button)
{
    sf::RectangleShape rect;
    rect.setSize({button.width, button.height});
    rect.setFillColor(toSfColor(button.backgroundColor));

    if (button.isSelected) {
        rect.setFillColor(toSfColor(button.selectedColor));
    }

    auto centerX = window.getSize().x / 2.0f + button.centerOffsetX - button.width / 2.0f;
    auto centerY = window.getSize().y / 2.0f + button.centerOffsetY - button.height / 2.0f;
    rect.setPosition({centerX, centerY});

    window.draw(rect);

    sf::Text text = toSfText(button.text);
    // Calc button text position based on button's position
    text.setPosition(sf::Vector2f(centerX + button.text.centerOffsetX, centerY + button.text.centerOffsetY));
    window.draw(text);
}

void Renderer::renderItem(sf::RenderWindow &window, const controller::Text &text)
{
    sf::Text t = toSfText(text);

    auto x = window.getSize().x / 2.0f + text.centerOffsetX;
    auto y = window.getSize().y / 2.0f + text.centerOffsetY;
    t.setPosition(sf::Vector2f(x, y));

    sf::Vector2<float> pos = t.getLocalBounds().getCenter();
    pos.x += text.originOffsetX;
    pos.y += text.originOffsetY;
    t.setOrigin(pos);

    window.draw(t);
}

} // namespace ui