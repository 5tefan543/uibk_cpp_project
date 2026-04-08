#pragma once

#include "color.hpp"
#include "view_item.hpp"
#include <vector>

namespace controller {

struct Card {
    Color backgroundColor = {150, 150, 150};
    float centerOffsetX = 0.0f;
    float centerOffsetY = 0.0f;
    float width = 400.0f;
    float height = 300.0f;
    std::vector<ViewItem> items;
};

} // namespace controller