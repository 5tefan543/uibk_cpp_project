#include "controller/view/color.hpp"
#include "controller/view/sprite.hpp"
#include "controller/view/text.hpp"
#include "ui/renderer.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Renderer converts controller color to SFML color")
{
    // ARRANGE
    ui::Renderer renderer;
    controller::Color color = {12, 34, 56};

    // ACT
    sf::Color sfColor = renderer.toSfColor(color);

    // ASSERT
    REQUIRE(sfColor.r == color.red);
    REQUIRE(sfColor.g == color.green);
    REQUIRE(sfColor.b == color.blue);
}

TEST_CASE("Renderer converts controller text to SFML text")
{
    // ARRANGE
    ui::Renderer renderer;
    controller::Text text;
    text.text = "Test text";
    text.size = 42;
    text.centerOffsetX = 10.0f;
    text.centerOffsetY = 20.0f;

    // ACT
    sf::Text sfText = renderer.toSfText(text);

    // ASSERT
    REQUIRE(sfText.getString() == sf::String("Test text"));
    REQUIRE(sfText.getCharacterSize() == 42);
    REQUIRE(sfText.getPosition().x == 10.0f);
    REQUIRE(sfText.getPosition().y == 20.0f);
}
