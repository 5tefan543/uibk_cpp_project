#include "shared/util.hpp"

controller::InputState createInputWithMouse(const geometry::Vec2<float> &position)
{
    controller::InputState input;
    input.mouseGrid = position;
    return input;
}
