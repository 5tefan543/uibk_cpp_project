#include "ui/ui.hpp"

#include "view/grid.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <imgui-SFML.h>
#include <iostream>

namespace ui {

UI::UI()
{
    std::cout << "UI constructed" << std::endl;
    initSfmlWindow();
    initImGuiSfml();
}

UI::~UI()
{
    std::cout << "UI destructed" << std::endl;
}

void UI::initSfmlWindow()
{
    // Fix resolution to same aspect ratio (16:9) as internal grid to maximize screen usage.
    // Non-multiple resolution-grid combination will create padding - no streching will occur.
    const unsigned width = (unsigned)view::gridWidth * 3;
    const unsigned height = (unsigned)view::gridHeight * 3;
    window_.create(sf::VideoMode({width, height}), "My Game", sf::State::Windowed);
    window_.setSize(sf::Vector2u(width, height));
    window_.setPosition({0, 0});
    window_.setFramerateLimit(60);
    setSfmlView();

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

void UI::setSfmlView()
{
    using view::gridHeight;
    using view::gridWidth;
    auto winSize = window_.getSize();

    const float widthScaled = gridWidth * ((float)winSize.x / gridWidth);
    const float heightScaled = gridHeight * ((float)winSize.y / gridHeight);

    const float widthOffset = (gridWidth - winSize.x) / 2;
    const float heightOffset = (gridHeight - winSize.y) / 2;

    // 1. Pull in more (blank,unused) space into view around our grid to keep gridWidth/Height consistent (View's size).
    // 2. Move view to align grid's and window's centers (View's position).
    // Always based on original view parameters and not last ones (e.g. window.getView()) which would throw of all
    // calculations
    auto v = sf::View(sf::FloatRect({widthOffset, heightOffset}, {widthScaled, heightScaled}));

    // 3. Zoom in/out such that the smaller of width/height matches window's corresponding dimension.
    // large content < 0 > small content (zoom <~> camera distance)
    v.zoom(std::max((float)gridWidth / winSize.x, (float)gridHeight / winSize.y));

    // setView() has to be called upon every change (render-target makes a copy of the view) src: "Using a
    // View" https://www.sfml-dev.org/tutorials/3.0/graphics/view/#defining-how-the-view-is-viewed
    window_.setView(v);
}

void UI::initImGuiSfml()
{
    if (!ImGui::SFML::Init(window_)) {
        throw std::runtime_error("Failed to initialize ImGui-SFML");
    }
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
    // 1. On window resize: adjust sfml view to prevent visual squishing/stretching
    if (inputState_.windowResized) {
        setSfmlView();
    }

    // 2. Start ImGui frame
    sf::Time deltaTime = imguiClock_.restart();
    fps_ = 1.0f / deltaTime.asSeconds();
    ImGui::SFML::Update(window_, deltaTime);

    // 3. Normal rendering
    window_.clear(renderer_.toSfColor(view.backgroundColor));
    renderer_.renderView(window_, view);

    // 4. Render debug UI on top
    debugUI_.render(inputState_, fps_);
    ImGui::SFML::Render(window_);

    // 5. Display everything
    window_.display();
}

} // namespace ui