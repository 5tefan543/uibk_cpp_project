#pragma once

namespace game {

struct HealthBarState {
    float previousHealth = -1.0f;   // -1 = uninitialized; detects first frame and damage events
    float initialRedBarNorm = 0.0f; // normalized red bar size at the moment damage was dealt (0..1)
    float redBarTimer = 0.0f;       // counts down from RED_FLASH_DURATION to 0

    static constexpr float RED_FLASH_DURATION = 0.8f;
};

} // namespace game
