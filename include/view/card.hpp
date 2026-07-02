#pragma once

#include "color.hpp"
#include "view_item.hpp"
#include <vector>

namespace view {

struct Card {
    Color backgroundColor = color::lightGray;
    geometry::Rectangle<float> rect = geometry::Rectangle<float>::centered(grid.getCenter(), grid.size / 2);
    std::vector<ViewElement> elements;
};

} // namespace view