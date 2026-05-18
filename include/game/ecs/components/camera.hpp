#pragma once

namespace game {

struct Camera {
    float x = 0.0f;        // World x-coordinate at the left edge of the visible screen
    float y = 0.0f;        // World y-coordinate at the top edge of the visible screen
    float margin = 128.0f; // extra margin around the edges of the map
};

} // namespace game
