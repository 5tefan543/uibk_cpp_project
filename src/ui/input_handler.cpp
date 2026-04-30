#include "ui/input_handler.hpp"
#include "controller/input/input_state.hpp"
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
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
    controller::InputState input{};
    ImGuiIO &imGuiIO = ImGui::GetIO();

    while (const std::optional event = window.pollEvent()) {
        ImGui::SFML::ProcessEvent(window, *event);

        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {

            if (keyPressed->code == sf::Keyboard::Key::F1) {
                input.toggleDebugPressed = true;
            }

            if (imGuiIO.WantCaptureKeyboard) {
                continue; // Skip processing game input if ImGui wants keyboard input
            }

            switch (keyPressed->code) {
            case sf::Keyboard::Key::Enter:
                input.confirmPressed = true;
                break;
            case sf::Keyboard::Key::Escape:
                input.cancelPressed = true;
                break;
            case sf::Keyboard::Key::Up:
            case sf::Keyboard::Key::W:
                input.upPressed = true;
                break;
            case sf::Keyboard::Key::Down:
            case sf::Keyboard::Key::S:
                input.downPressed = true;
                break;
            case sf::Keyboard::Key::Left:
            case sf::Keyboard::Key::A:
                input.leftPressed = true;
                break;
            case sf::Keyboard::Key::Right:
            case sf::Keyboard::Key::D:
                input.rightPressed = true;
                break;
            default:
                break;
            }
        }

        if (const auto *mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {

            if (imGuiIO.WantCaptureMouse) {
                continue; // Skip processing game input if ImGui wants mouse input
            }

            switch (mouseButtonPressed->button) {
            case sf::Mouse::Button::Left:
                input.mouseLeftPressed = true;
                break;
            case sf::Mouse::Button::Right:
                input.mouseRightPressed = true;
                break;
            case sf::Mouse::Button::Middle:
                input.mouseMiddlePressed = true;
                break;
            default:
                break;
            }
        }

        if (event->is<sf::Event::MouseMoved>()) {

            if (imGuiIO.WantCaptureMouse) {
                continue; // Skip processing game input if ImGui wants mouse input
            }

            input.mouseMoved = true;
        }
    }

    if (!imGuiIO.WantCaptureKeyboard) {
        input.upHeld =
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);

        input.downHeld =
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);

        input.leftHeld =
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);

        input.rightHeld =
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
    }

    if (!imGuiIO.WantCaptureMouse) {
        input.mouseLeftHeld = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        input.mouseRightHeld = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
        input.mouseMiddleHeld = sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle);
    }

    const sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
    input.mouseX = mousePosition.x;
    input.mouseY = mousePosition.y;

    const sf::Vector2u windowSize = window.getSize();
    input.windowWidth = windowSize.x;
    input.windowHeight = windowSize.y;

    return input;
}

} // namespace ui