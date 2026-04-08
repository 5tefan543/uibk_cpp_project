#include "ui/input_handler.hpp"
#include "controller/input/input_state.hpp"
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
    controller::InputState state{};

    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            switch (keyPressed->code) {
            case sf::Keyboard::Key::Enter:
                state.confirm = true;
                break;
            case sf::Keyboard::Key::Escape:
                state.pause = true;
                break;
            case sf::Keyboard::Key::Up:
            case sf::Keyboard::Key::W:
                state.up = true;
                break;
            case sf::Keyboard::Key::Down:
            case sf::Keyboard::Key::S:
                state.down = true;
                break;
            case sf::Keyboard::Key::Left:
            case sf::Keyboard::Key::A:
                state.left = true;
                break;
            case sf::Keyboard::Key::Right:
            case sf::Keyboard::Key::D:
                state.right = true;
                break;
            default:
                break;
            }
        }

        if (const auto *mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            switch (mouseButtonPressed->button) {
            case sf::Mouse::Button::Left:
                state.mouseLeft = true;
                break;
            case sf::Mouse::Button::Right:
                state.mouseRight = true;
                break;
            case sf::Mouse::Button::Middle:
                state.mouseMiddle = true;
                break;
            default:
                break;
            }
        }
    }

    state.mouseX = sf::Mouse::getPosition(window).x;
    state.mouseY = sf::Mouse::getPosition(window).y;

    return state;
}

} // namespace ui