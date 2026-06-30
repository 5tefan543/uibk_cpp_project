#include "ui/ui.hpp"
#include "logging/log.hpp"
#include "view/grid.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <imgui-SFML.h>
namespace ui {

UI::UI()
{
    logger::log(logger::DEBUG, "UI constructed");
    initSfmlWindow();
    initImGuiSfml();
}

UI::~UI()
{
    logger::log(logger::DEBUG, "UI destructed");
}

void UI::initSfmlWindow()
{
    sf::VideoMode videoMode({(unsigned)view::grid.size.x, (unsigned)view::grid.size.y});
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

void UI::setSfmlView(geometry::Vec2<float> cameraPosition)
{
    sf::View view;
    view.setSize(toSFML(view::grid.size));
    view.setCenter(toSFML((view::grid.size / 2.0f) + cameraPosition));
    view.setViewport(getLetterboxViewport());
    window_.setView(view);
}

sf::FloatRect UI::getLetterboxViewport() const
{
    const sf::Vector2u windowSize = window_.getSize();

    const float windowRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    const float viewRatio = view::grid.size.x / view::grid.size.y;

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

void UI::render(const view::View &view, const controller::timeDelta &dt)
{
    // 1. Start ImGui frame
    ImGui::SFML::Update(window_, std::chrono::duration_cast<std::chrono::microseconds>(dt));

    // 2. Normal rendering
    window_.clear(renderer_.toSfColor(view.backgroundColor));
    renderView(window_, view);

    // 3. Render debug UI on top
    setSfmlView(view.cameraPosition);
    renderer_.renderDebugContext(window_);
    debugUI_.render(inputState_, dt);
    ImGui::SFML::Render(window_);

    // 4. Display everything
    window_.display();

    // 5. Set view to camera-relative for next frame's input polling
    // Otherwise mouse input is not correctly mapped to grid coordinates when camera is moved
    setSfmlView(view.cameraPosition);
}

void UI::renderView(sf::RenderWindow &window, const view::View &view)
{
    std::optional<view::ViewMode> currentViewMode;

    for (const auto &node : view.nodes) {
        if (currentViewMode != node.mode) {
            currentViewMode = node.mode;
            if (node.mode == view::ViewMode::FixedToWorld) {
                setSfmlView(view.cameraPosition);
            } else {
                setSfmlView({0.0f, 0.0f});
            }
        }

        renderer_.renderViewElement(window, node.element);
    }
}

} // namespace ui