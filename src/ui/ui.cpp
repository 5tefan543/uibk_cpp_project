#include "ui/ui.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <imgui-SFML.h>
#include <iostream>
#include <optional>

namespace ui {

UI::UI()
{
    std::cout << "UI constructed" << std::endl;
    initWindow();
    initImGuiSfml(window);
}

UI::~UI()
{
    std::cout << "UI destructed" << std::endl;
    ImGui::SFML::Shutdown();
}

void UI::initWindow()
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    window.create(desktop, "My Game");
    window.setPosition({0, 0});
    window.setFramerateLimit(60);
}

void UI::initImGuiSfml(sf::RenderWindow &window)
{
    if (!ImGui::SFML::Init(window)) {
        throw std::runtime_error("Failed to initialize ImGui-SFML");
    }
}

bool UI::isOpen() const
{
    return window.isOpen();
}

controller::InputState UI::pollInput()
{
    return inputHandler.pollInput(window);
}

void UI::render(const controller::View &view, controller::DebugState &debugState)
{
    // 1. Start ImGui frame
    ImGui::SFML::Update(window, imguiClock.restart());

    // 2. Normal rendering
    window.clear(renderer.toSfColor(view.backgroundColor));
    renderer.renderItems(window, view.items);

    // 3. Render debug UI on top
    debugUI.render(debugState);
    ImGui::SFML::Render(window);

    // 4. Display everything
    window.display();
}

} // namespace ui