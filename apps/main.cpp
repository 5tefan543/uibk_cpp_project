#include "controller/controller.hpp"
#include "logging/log.hpp"
#include "ui/ui.hpp"
#include "view/view.hpp"
#include <atomic>
#include <controller/persistence/persistence_manager.hpp>
#include <csignal>
#include <cstdlib>
#include <exception>

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

        // Load config and configure logger before anything else to ensure all logs are properly handled
        controller::GameConfig config = controller::PersistenceManager::getConfig();
        logger::configure(config.logConfig.level, config.logConfig.useColor);

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
        logger::log(logger::ERROR, std::format("Fatal error: {}", e.what()));
        return EXIT_FAILURE;
    }
}