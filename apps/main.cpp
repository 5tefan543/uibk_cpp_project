#include "controller/controller.hpp"
#include "controller/view/view.hpp"
#include "ui/ui.hpp"
#include <cstdlib>

int main()
{
    controller::Controller controller;
    ui::UI ui;

    const float fixed_dt = 1.0f / 60.0f; // Fixed time step for updates

    while (ui.isOpen()) {
        controller::InputState input = ui.pollInput();
        controller.update(input, fixed_dt);
        controller::BaseState &currentState = controller.getCurrentState();
        controller::View view = currentState.getView();
        ui.render(view);
    }

    return EXIT_SUCCESS;
}