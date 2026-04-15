#include "controller/controller.hpp"
#include "controller/view/view.hpp"
#include "ui/ui.hpp"
#include <atomic>
#include <csignal>
#include <cstdlib>

std::atomic<bool> shutdown_requested(false);

void signal_handler(int signal)
{
    if (signal == SIGTERM || signal == SIGINT) {
        shutdown_requested = true;
    }
}

int main()
{
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler);

    controller::Controller controller;
    ui::UI ui;

    const float fixed_dt = 1.0f / 60.0f; // Fixed time step for updates

    while (ui.isOpen() && !shutdown_requested) {
        controller::InputState input = ui.pollInput();

        controller.update(input, fixed_dt);
        controller::BaseState &currentState = controller.getCurrentState();
        if (typeid(currentState) == typeid(controller::ExitState)) {
            break;
        }
        controller::View view = currentState.getView();
        controller::DebugState &debug = controller.getDebugState();

        ui.render(view, debug);
    }

    return EXIT_SUCCESS;
}