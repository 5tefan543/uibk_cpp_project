#pragma once

#include "geometry/vector.hpp"

namespace controller {

struct InputState {
    bool upPressed = false;
    bool downPressed = false;
    bool leftPressed = false;
    bool rightPressed = false;

    bool upHeld = false;
    bool downHeld = false;
    bool leftHeld = false;
    bool rightHeld = false;

    bool mouseLeftPressed = false;
    bool mouseRightPressed = false;
    bool mouseMiddlePressed = false;

    bool mouseLeftHeld = false;
    bool mouseRightHeld = false;
    bool mouseMiddleHeld = false;

    bool mouseMoved = false;
    geometry::Vec2<float> mouseGrid{0, 0};

    bool confirmPressed = false;
    bool cancelPressed = false;
    bool controlHeld = false;

    bool toggleDebugPressed = false;
};

} // namespace controller