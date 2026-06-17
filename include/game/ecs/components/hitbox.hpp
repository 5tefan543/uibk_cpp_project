#pragma once

#include "geometry/rectangle.hpp"

namespace game {
struct HitBox {
    Vec2<float> offset;
    Vec2<float> size;
};
} // namespace game