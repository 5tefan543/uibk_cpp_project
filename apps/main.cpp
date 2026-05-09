#include "controller/controller.hpp"
#include "ui/ui.hpp"
#include "view/view.hpp"
#include <atomic>
#include <csignal>
#include <cstdlib>

std::atomic<bool> shutdownRequested(false);

void signalHandler(int signal)
{
    if (signal == SIGTERM || signal == SIGINT) {
        shutdownRequested = true;
    }
}

int main()
{
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGINT, signalHandler);

    controller::Controller controller;
    ui::UI ui;

    const float fixedDt = 1.0f / 60.0f; // Fixed time step for updates

    while (ui.isOpen() && !shutdownRequested) {
        const controller::InputState input = ui.pollInput();

        controller.update(input, fixedDt);
        controller::BaseState &currentState = controller.getCurrentState();
        if (typeid(currentState) == typeid(controller::ExitState)) {
            break;
        }
        const view::View &view = currentState.getView();
        controller::DebugContext &debug = controller.getDebugContext();
        ui.render(view, debug);
    }

    return EXIT_SUCCESS;
}