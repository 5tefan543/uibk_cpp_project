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

    int mouseX = 0;
    int mouseY = 0;

    float windowWidth = 1920.0f;
    float windowHeight = 1080.0f;

    bool confirmPressed = false;
    bool cancelPressed = false;

    bool toggleDebugPressed = false;
};

} // namespace controller