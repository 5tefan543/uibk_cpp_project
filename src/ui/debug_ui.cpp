#include "ui/debug_ui.hpp"
#include <imgui.h>
#include <iostream>
#include <vector>

namespace ui {

DebugUI::DebugUI()
{
    std::cout << "DebugUI constructed" << std::endl;
}

DebugUI::~DebugUI()
{
    std::cout << "DebugUI destructed" << std::endl;
}

void DebugUI::render(controller::DebugState &debugState, const controller::InputState &inputState, float fps)
{
    if (!debugState.active) {
        return;
    }

    // Set the next window position before calling Begin()
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);
    ImGui::Begin("Debug", &debugState.active, ImGuiWindowFlags_AlwaysAutoResize);

    float smoothedFPS = (prevFPS > 0.0f) ? 0.99f * prevFPS + 0.01f * fps : fps;
    prevFPS = smoothedFPS;
    float frameTime = smoothedFPS > 0.0f ? 1000.0f / smoothedFPS : 0.0f;

    if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("FPS: %.0f", smoothedFPS);
        ImGui::Text("Frame time: %.0f ms", frameTime);
        ImGui::Text("Mouse position: (%d, %d)", inputState.mouseX, inputState.mouseY);
        ImGui::TextUnformatted(debugState.currentStates.c_str());
    }

    if (ImGui::CollapsingHeader("Game Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SeparatorText("Stage / Wave");
        ImGui::InputInt("Stage", &debugState.gameDebugState.stage);
        ImGui::InputInt("Wave", &debugState.gameDebugState.wave);
        if (ImGui::Button("Reload Stage/Wave")) {
            debugState.gameDebugState.isStageWaveReloadRequested = true;
        }

        ImGui::SeparatorText("Collision");
        ImGui::Checkbox("Show Hitboxes", &debugState.gameDebugState.showHitboxes);
    }

    if (ImGui::CollapsingHeader("Entities", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char *preview = "Select entity";

        if (debugState.gameDebugState.selectedEntity.has_value()) {
            static std::string previewStr;
            previewStr = "Entity " + std::to_string(debugState.gameDebugState.selectedEntity.value());
            preview = previewStr.c_str();
        }

        if (ImGui::BeginCombo("Entity", preview)) {
            for (const auto &entity : debugState.gameDebugState.entities) {
                bool isSelected = debugState.gameDebugState.selectedEntity.has_value()
                                  && debugState.gameDebugState.selectedEntity.value() == entity;

                std::string label = "Entity " + std::to_string(entity);

                if (ImGui::Selectable(label.c_str(), isSelected)) {
                    debugState.gameDebugState.selectedEntity = entity;
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }
    }

    ImGui::End();
}

} // namespace ui