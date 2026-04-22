#pragma once

#include "controller/state/state.hpp"

inline constexpr float dummyDeltaTime = 0.016f;

enum INPUT { UP, DOWN, CONFIRM, NONE, LEFT, RIGHT };

controller::StateTransitionAction applyInput(std::unique_ptr<controller::MenuState> &state, const INPUT in);