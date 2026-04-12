#include "ui/debug_ui.hpp"
#include <imgui.h>
#include <iostream>

namespace ui {

DebugUI::DebugUI()
{
    std::cout << "DebugUI constructed" << std::endl;
}

DebugUI::~DebugUI()
{
    std::cout << "DebugUI destructed" << std::endl;
}

void DebugUI::render(controller::DebugState &debugState, const char *windowTitle)
{
    if (!debugState.active) {
        return;
    }

    ImGui::Begin(windowTitle, &debugState.active);

    ImGui::Checkbox("Show FPS", &debugState.showFPS);
    ImGui::Checkbox("Show Hitboxes", &debugState.showHitboxes);

    ImGui::End();
}

} // namespace ui