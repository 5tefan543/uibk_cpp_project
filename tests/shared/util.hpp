#pragma once

#include "controller/state/state.hpp"

inline constexpr float dummyDeltaTime = 0.016f;

enum INPUT { UP, DOWN, CONFIRM, NONE, LEFT, RIGHT };

controller::InputState createInputWithMouse(const geometry::Vec2<float> &position);

template <typename T>
controller::StateTransitionAction applyInput(std::unique_ptr<T> &state, const INPUT in)
{
    // ARRANGE
    controller::InputState input;
    switch (in) {
    case UP:
        input.upPressed = true;
        break;
    case DOWN:
        input.downPressed = true;
        break;
    case CONFIRM:
        input.confirmPressed = true;
        break;
    case NONE:
        break;
    case LEFT:
        input.leftPressed = true;
        break;
    case RIGHT:
        input.rightPressed = true;
        break;
    }

    // ACT
    return state->update(input, dummyDeltaTime);
}

template <typename T>
controller::StateTransitionAction applyMouseMove(std::unique_ptr<T> &state, const geometry::Vec2<float> &position)
{
    controller::InputState input = createInputWithMouse(position);
    input.mouseMoved = true;

    return state->update(input, dummyDeltaTime);
}

template <typename T>
controller::StateTransitionAction applyMouseClick(std::unique_ptr<T> &state, geometry::Vec2<float> position)
{
    controller::InputState input = createInputWithMouse(position);
    input.mouseLeftPressed = true;

    return state->update(input, dummyDeltaTime);
}

class ViewElementAccessor {
  public:
    template <typename T>
    static T &as(const view::ViewElement &item)
    {
        return std::get<std::reference_wrapper<T>>(item).get();
    }

    template <typename T>
    static bool is(const view::ViewElement &item)
    {
        return std::holds_alternative<std::reference_wrapper<T>>(item);
    }
};