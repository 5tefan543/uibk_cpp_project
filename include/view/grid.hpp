#pragma once

#include "geometry/rectangle.hpp"

namespace view {

// Virtual, internal grid for positioning stuff like view Items.
// Grid based coordinates will be projected onto current window dimensions for every render pass.
// 0,0 position in order to be identical to SFML's coordiante system.
constexpr geometry::Rectangle<float> grid = {.position = {0, 0}, .size = {1920, 1080}};

} // namespace view