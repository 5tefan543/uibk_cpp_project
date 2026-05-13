#include "controller/controller.hpp"
#include "ui/ui.hpp"
#include "view/view.hpp"
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>

std::atomic<bool> shutdownRequested(false);

void signalHandler(int signal)
{
    if (signal == SIGTERM || signal == SIGINT) {
        shutdownRequested = true;
    }
}

int main()
{
    try {
        std::signal(SIGTERM, signalHandler);
        std::signal(SIGINT, signalHandler);

        controller::Controller controller;
        ui::UI ui;

        const float fixedDt = 1.0f / 60.0f; // Fixed time step for updates

        while (ui.isOpen() && !shutdownRequested) {
            const controller::InputState &input = ui.pollInput();

            controller.update(input, fixedDt);
            controller::BaseState &currentState = controller.getCurrentState();
            if (typeid(currentState) == typeid(controller::ExitState)) {
                break;
            }
            const view::View &view = currentState.getView();
            ui.render(view);
        }
        return EXIT_SUCCESS;
    } catch (const std::exception &e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}