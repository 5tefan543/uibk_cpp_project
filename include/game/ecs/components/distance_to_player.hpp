#pragma once

#include <limits>

namespace game {

struct DistanceToPlayer {
    float value = std::numeric_limits<float>::infinity();
    bool hasPlayer = false;
};

} // namespace game