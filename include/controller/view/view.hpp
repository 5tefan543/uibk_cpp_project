#pragma once

#include "controller/view/card.hpp"
#include "controller/view/color.hpp"
#include "controller/view/view_item.hpp"
#include <vector>

namespace controller {
struct View {
    Color backgroundColor = {20, 20, 20};
    std::vector<ViewItem> items;
};
} // namespace controller