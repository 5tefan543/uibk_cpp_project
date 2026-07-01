#pragma once

#include <chrono>

namespace controller {

using timeDelta = std::chrono::steady_clock::duration;

inline float toSeconds(const timeDelta &dt)
{
    return std::chrono::duration<double, std::milli>(dt).count() / (double)1e3;
}
} // namespace controller