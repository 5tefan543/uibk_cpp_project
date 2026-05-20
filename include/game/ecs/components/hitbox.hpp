#pragma once

#include "shapes/rectangle.hpp"

namespace game {
struct HitBox {
    Rectangle<float> rect;
    bool isActive;
};
} // namespace game