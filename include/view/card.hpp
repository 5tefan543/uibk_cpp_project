#pragma once

#include "color.hpp"
#include "view_item.hpp"
#include <vector>

namespace view {

struct Card {
    Color backgroundColor = {150, 150, 150};
    geometry::Rectangle<float> rect = geometry::Rectangle<float>::centered(grid.getCenter(), grid.size / 2);
    std::vector<ViewElement> elements;
};

} // namespace view