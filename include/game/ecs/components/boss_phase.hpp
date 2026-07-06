#pragma once

namespace game {

enum class BossPhaseState {
    Phase1,
    Phase2,
};

struct BossPhase {
    BossPhaseState phase = BossPhaseState::Phase1;
};

} // namespace game