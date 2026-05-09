#pragma once

#include "controller/state/state.hpp"

inline constexpr float dummyDeltaTime = 0.016f;

enum INPUT { UP, DOWN, CONFIRM, NONE, LEFT, RIGHT };

controller::StateTransitionAction applyInput(std::unique_ptr<controller::MenuState> &state, const INPUT in);

controller::InputState createInputWithMouse(float gridX, float gridY);

controller::StateTransitionAction applyMouseMove(std::unique_ptr<controller::MenuState> &state, float gridX,
                                                 float gridY);

controller::StateTransitionAction applyMouseClick(std::unique_ptr<controller::MenuState> &state, float gridX,
                                                  float gridY);

class ViewItemAccessor {
  public:
    template <typename T>
    static T &as(const controller::ViewItem &item)
    {
        return std::get<std::reference_wrapper<T>>(item).get();
    }

    template <typename T>
    static bool is(const controller::ViewItem &item)
    {
        return std::holds_alternative<std::reference_wrapper<T>>(item);
    }
};