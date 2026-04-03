#include "ui/renderer.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>

namespace ui {

Renderer::Renderer()
{
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

void Renderer::renderItems(sf::RenderWindow &window, const std::vector<controller::ViewItem> &items)
{
    for (const controller::ViewItem &item : items) {
        std::visit([this, &window](const auto &item) { renderItem(window, item); }, item);
    }
}

void Renderer::renderItem(sf::RenderWindow &window, const std::unique_ptr<controller::Card> &card)
{
    // Render card first
    sf::RectangleShape rect;
    rect.setSize({card->width, card->height});
    rect.setFillColor(toSfColor(card->backgroundColor));
    auto centerX = window.getSize().x / 2.0f + card->centerOffsetX - card->width / 2.0f;
    auto centerY = window.getSize().y / 2.0f + card->centerOffsetY - card->height / 2.0f;
    rect.setPosition({centerX, centerY});
    window.draw(rect);

    // Render items on the card
    renderItems(window, card->items);
}

void Renderer::renderItem(sf::RenderWindow &window, const controller::Button &button)
{
    sf::RectangleShape rect;
    rect.setSize({button.width, button.height});
    rect.setFillColor(toSfColor(button.backgroundColor));
    auto centerX = window.getSize().x / 2.0f + button.centerOffsetX - button.width / 2.0f;
    auto centerY = window.getSize().y / 2.0f + button.centerOffsetY - button.height / 2.0f;
    rect.setPosition({centerX, centerY});
    window.draw(rect);
}

} // namespace ui