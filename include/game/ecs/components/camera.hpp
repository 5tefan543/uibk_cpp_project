#pragma once

namespace game {

struct Camera {
    float x = 0.0f; // Camera position (follows player)
    float y = 0.0f;
    float viewportWidth = 960.0f;  // Viewport size (1920 / 2 scale factor)
    float viewportHeight = 540.0f; // (1080 / 2 scale factor)
};

} // namespace game
