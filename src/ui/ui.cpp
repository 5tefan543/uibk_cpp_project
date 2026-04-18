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
    initSfmlWindow();
    initImGuiSfml(window_);
}

UI::~UI()
{
    std::cout << "UI destructed" << std::endl;
}

void UI::initSfmlWindow()
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    window_.create(desktop, "My Game");
    window_.setPosition({0, 0});
    window_.setFramerateLimit(60);
}

void UI::initImGuiSfml(sf::RenderWindow &window)
{
    if (!ImGui::SFML::Init(window)) {
        throw std::runtime_error("Failed to initialize ImGui-SFML");
    }
}

bool UI::isOpen() const
{
    return window_.isOpen();
}

controller::InputState UI::pollInput()
{
    inputState_ = inputHandler_.pollInput(window_);
    return inputState_;
}

void UI::render(const controller::View &view, controller::DebugContext &debug)
{
    // 1. Start ImGui frame
    sf::Time deltaTime = imguiClock_.restart();
    fps_ = 1.0f / deltaTime.asSeconds();
    ImGui::SFML::Update(window_, deltaTime);

    // 2. Normal rendering
    window_.clear(renderer_.toSfColor(view.backgroundColor));
    renderer_.renderItems(window_, view.items);

    // 3. Render debug UI on top
    debugUI_.render(debug, inputState_, fps_);
    ImGui::SFML::Render(window_);

    // 4. Display everything
    window_.display();
}

} // namespace ui