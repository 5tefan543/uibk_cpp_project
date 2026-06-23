#pragma once

#include "geometry/rectangle.hpp"

namespace game {
struct HitBox {
    geometry::Vec2<float> offset;
    geometry::Vec2<float> size;
};
} // namespace game