#include "ui/renderer.hpp"
#include "controller/debug/debug_context.hpp"
#include "view/grid.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>

namespace ui {

Renderer::Renderer()
{
    // Load all fonts from disk once upon instantiation
    fonts_ = std::vector<sf::Font>({sf::Font("assets/font/BigBlueTerm_Nerd_Font/BigBlueTerm437NerdFont-Regular.ttf")});
#ifdef LOG_STDOUT
    std::cout << "Renderer constructed" << std::endl;
#endif
}

Renderer::~Renderer()
{
#ifdef LOG_STDOUT
    std::cout << "Renderer destructed" << std::endl;
#endif
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

void Renderer::renderDebugLocationTable(sf::RenderWindow &window)
{
    auto &context = controller::DebugContext::get();
    if (!context.gameSettings.showLocationTable) {
        return;
    }
    game::LocationTable &lt = context.gameSession->locationTable;
    for (unsigned y = 0; y < lt.numBuckets.y; y++) {
        for (unsigned x = 0; x < lt.numBuckets.x; x++) {
            sf::RectangleShape bucket;
            bucket.setPosition({(x * lt.bucketSize.x), (y * lt.bucketSize.y)});
            bucket.setSize({lt.bucketSize.x, lt.bucketSize.y});
            bucket.setOutlineColor(sf::Color::Blue);
            bucket.setFillColor(sf::Color::Transparent);
            bucket.setOutlineThickness(1.0f);
            window.draw(bucket);

            sf::Text t(toSfFont(view::Font::Default), std::format("{}", lt.cgetBucket(x, y)->size()));
            t.setOrigin(t.getLocalBounds().getCenter());
            t.setPosition({bucket.getPosition() + bucket.getGeometricCenter()});
            t.setFillColor(sf::Color::Black);
            const float twscale = lt.bucketSize.x / t.getLocalBounds().size.x;
            const float thscale = lt.bucketSize.y / t.getLocalBounds().size.y;
            const auto tscale = std::min(twscale * 0.7f, thscale * 0.7f);
            t.setScale({tscale, tscale});
            window.draw(t);
        }
    }
}

void Renderer::renderDebugContext(sf::RenderWindow &window)
{
    renderDebugLocationTable(window);
    // Add more debug related rendering here if required
}

} // namespace ui