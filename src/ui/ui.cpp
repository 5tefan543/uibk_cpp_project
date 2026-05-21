#include "ui/ui.hpp"

#include "view/grid.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <imgui-SFML.h>
#include <iostream>

namespace ui {

UI::UI()
{
#ifdef LOG_STDOUT
    std::cout << "UI constructed" << std::endl;
#endif
    initSfmlWindow();
    initImGuiSfml();
}

UI::~UI()
{
#ifdef LOG_STDOUT
    std::cout << "UI destructed" << std::endl;
#endif
}

void UI::initSfmlWindow()
{
    sf::VideoMode videoMode({(unsigned)view::gridWidth, (unsigned)view::gridHeight});
    window_.create(videoMode, "My Game", sf::State::Windowed);
    window_.setPosition({0, 0});
    window_.setFramerateLimit(60);

    // Print all available video modes - maybe interesting later if we want to support multiple resolutions with same
    // ratio as internal grid

    // std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes(); for (std::size_t i = 0; i <
    // modes.size(); ++i) {
    //     sf::VideoMode mode = modes[i];
    //     std::cout << "Mode #" << i << ": " << mode.size.x << "x" << mode.size.y << " - " << mode.bitsPerPixel <<
    //     "bpp" << std::endl;
    // }
}

void UI::initImGuiSfml()
{
    if (!ImGui::SFML::Init(window_)) {
        throw std::runtime_error("Failed to initialize ImGui-SFML");
    }
}

void UI::setSfmlView(float cameraX, float cameraY)
{
    sf::View view;
    view.setSize({view::gridWidth, view::gridHeight});
    view.setCenter({view::gridWidth / 2.0f + cameraX, view::gridHeight / 2.0f + cameraY});
    view.setViewport(getLetterboxViewport());
    window_.setView(view);
}

sf::FloatRect UI::getLetterboxViewport() const
{
    const sf::Vector2u windowSize = window_.getSize();

    const float windowRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    const float viewRatio = view::gridWidth / view::gridHeight;

    // Viewport uses normalized coordinates [0, 1]
    // Start with entire window
    float viewportWidth = 1.0f;
    float viewportHeight = 1.0f;
    float viewportX = 0.0f;
    float viewportY = 0.0f;

    if (windowRatio > viewRatio) {
        // Window is wider than the game view -> vertical bars left/right.
        viewportWidth = viewRatio / windowRatio;   // Calculate width based on height to maintain aspect ratio
        viewportX = (1.0f - viewportWidth) / 2.0f; // Center horizontally
    } else {
        // Window is taller than the game view -> horizontal bars top/bottom.
        viewportHeight = windowRatio / viewRatio;   // Calculate height based on width to maintain aspect ratio
        viewportY = (1.0f - viewportHeight) / 2.0f; // Center vertically
    }

    return sf::FloatRect({viewportX, viewportY}, {viewportWidth, viewportHeight});
}

bool UI::isOpen() const
{
    return window_.isOpen();
}

const controller::InputState &UI::pollInput()
{
    inputState_ = inputHandler_.pollInput(window_);
    return inputState_;
}

void UI::render(const view::View &view)
{
    // 1. Start ImGui frame
    sf::Time deltaTime = imguiClock_.restart();
    fps_ = 1.0f / deltaTime.asSeconds();
    ImGui::SFML::Update(window_, deltaTime);

    // 2. Normal rendering
    window_.clear(renderer_.toSfColor(view.backgroundColor));
    renderView(window_, view);

    // 3. Render debug UI on top
    debugUI_.render(inputState_, fps_);
    ImGui::SFML::Render(window_);

    // 4. Display everything
    window_.display();

    // 5. Set view to camera-relative for next frame's input polling
    // Otherwise mouse input is not correctly mapped to grid coordinates when camera is moved
    setSfmlView(view.cameraX, view.cameraY);
}

void UI::renderView(sf::RenderWindow &window, const view::View &view)
{
    std::optional<view::ViewMode> currentViewMode;

    for (const auto &node : view.nodes) {
        if (currentViewMode != node.mode) {
            currentViewMode = node.mode;
            if (node.mode == view::ViewMode::FixedToWorld) {
                setSfmlView(view.cameraX, view.cameraY);
            } else {
                setSfmlView(0.0f, 0.0f);
            }
        }

        renderer_.renderViewElement(window, node.element);
    }
}

} // namespace ui