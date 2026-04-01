#include "ui/input_handler.hpp"
#include "controller/input_state.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>

namespace ui {

InputHandler::InputHandler()
{
    std::cout << "InputHandler constructed" << std::endl;
}

InputHandler::~InputHandler()
{
    std::cout << "InputHandler destructed" << std::endl;
}

controller::InputState InputHandler::pollInput(sf::RenderWindow &window)
{
    controller::InputState state;
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }
    }

    // debug only: should this be in event loop or here?
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        window.close();
    }

    state.left =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
    state.right =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
    state.up = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
    state.down =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
    state.mouseLeft = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    state.mouseRight = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
    state.mouseMiddle = sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle);
    state.mouseX = sf::Mouse::getPosition(window).x;
    state.mouseY = sf::Mouse::getPosition(window).y;
    return state;
}

} // namespace ui