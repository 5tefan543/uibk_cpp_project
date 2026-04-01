#pragma once

namespace controller {

struct InputState {
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;

    bool mouseLeft = false;
    bool mouseRight = false;
    bool mouseMiddle = false;

    int mouseX = 0;
    int mouseY = 0;

    bool confirm = false;
    bool pause = false;
};

} // namespace controller