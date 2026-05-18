#pragma once

namespace view {

// Virtual, internal grid for positioning stuff like view Items.
// Grid based coordinates will be projected onto current window dimensions for every render pass.
constexpr const float gridWidth = 1920;
constexpr const float gridHeight = 1080;

} // namespace view