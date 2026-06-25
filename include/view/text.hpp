#pragma once

#include "color.hpp"
#include "font.hpp"
#include "grid.hpp"
#include <string>

namespace view {

enum class TextAlignment { Left, Center, Right };

struct Text {
    std::string text = std::string();
    FontType font = FontType::Default;
    unsigned int size = 30;
    Color color = {255, 255, 255};
    geometry::Vec2<float> position = grid.getCenter();
    geometry::Vec2<float> originOffset = {0.0f, 0.0f};
    TextAlignment alignment = TextAlignment::Center;
};

} // namespace view