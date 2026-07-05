#pragma once

namespace game {

struct PlayerAttackCooldown {
    float attackDurationSec = 0.0f;
    float attackRemainingSec = 0.0f;

    float specialAttackDurationSec = 0.0f;
    float specialAttackRemainingSec = 0.0f;
};

} // namespace game