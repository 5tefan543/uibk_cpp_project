#pragma once

#include <string>

namespace controller {

struct Button {
    std::string text;
    float centerOffsetX = 0.0f;
    float centerOffsetY = 0.0f;
    float width = 100.0f;
    float height = 50.0f;
};

} // namespace controller