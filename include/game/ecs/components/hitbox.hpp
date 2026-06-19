#pragma once

#include "geometry/rectangle.hpp"

namespace game {
struct HitBox {
    // rect.position <-> used as offset
    geometry::Rectangle<float> rect;
};
} // namespace game