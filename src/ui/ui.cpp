#include "ui/ui.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <optional>

namespace ui {

UI::UI()
{
    std::cout << "UI constructed" << std::endl;
    initWindow();
}

UI::~UI()
{
    std::cout << "UI destructed" << std::endl;
}

void UI::initWindow()
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    window.create(desktop, "My Game");
    window.setPosition({0, 0});
    window.setFramerateLimit(60);
}

bool UI::isOpen() const
{
    return window.isOpen();
}

controller::InputState UI::pollInput()
{
    return input_handler.pollInput(window);
}

void UI::render(const controller::View &view)
{
    window.clear(renderer.toSfColor(view.backgroundColor));
    renderer.renderItems(window, view.items);
    window.display();
}

} // namespace ui