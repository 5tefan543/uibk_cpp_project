#pragma once

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
    float mouseGridX = 0.0;
    float mouseGridY = 0.0;

    bool confirmPressed = false;
    bool cancelPressed = false;

    bool toggleDebugPressed = false;

    bool windowResized = false;
};

} // namespace controller