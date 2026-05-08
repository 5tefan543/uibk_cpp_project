#include "ui/ui.hpp"

#include "controller/view/grid.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <imgui-SFML.h>
#include <iostream>

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
    // Fix resolution to same aspect ratio (16:9) as internal grid to maximize screen usage.
    // Non-multiple resolution-grid combination will create padding - no streching will occur.
    const unsigned width = (unsigned)controller::gridWidth;
    const unsigned height = (unsigned)controller::gridHeight;
    window_.create(sf::VideoMode({width, height}), "My Game");
    window_.setSize(sf::Vector2u(width, height));
    window_.setPosition({0, 0});
    window_.setFramerateLimit(60);

    // Print all available video modes - maybe interesting later if we want to support multiple resolutions with same
    // ratio as internal grid
    //
    // std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes(); for (std::size_t i = 0; i <
    // modes.size(); ++i) {
    //     sf::VideoMode mode = modes[i];
    //     std::cout << "Mode #" << i << ": " << mode.size.x << "x" << mode.size.y << " - " << mode.bitsPerPixel << "
    //     bpp"
    //               << std::endl;
    // }
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
    return inputHandler_.pollInput(window_);
}

void UI::render(const controller::View &view, controller::DebugContext &debug, const bool windowResized)
{
    // 1. Start ImGui frame
    sf::Time deltaTime = imguiClock_.restart();
    fps_ = 1.0f / deltaTime.asSeconds();
    ImGui::SFML::Update(window_, deltaTime);

    // 2. Normal rendering
    window_.clear(renderer_.toSfColor(view.backgroundColor));
    renderer_.renderView(window_, view, windowResized);

    // 3. Render debug UI on top
    debugUI_.render(debug, inputState_, fps_);
    ImGui::SFML::Render(window_);

    // 4. Display everything
    window_.display();
}

} // namespace ui