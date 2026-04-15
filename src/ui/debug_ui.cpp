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

void DebugUI::render(controller::DebugState &debug, const controller::InputState &input, float fps)
{
    if (!debug.active) {
        return;
    }

    // Set the next window position before calling Begin()
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);
    ImGui::Begin("Debug", &debug.active, ImGuiWindowFlags_AlwaysAutoResize);

    float smoothedFps = (prevFps > 0.0f) ? 0.99f * prevFps + 0.01f * fps : fps;
    prevFps = smoothedFps;
    float frameTimeMs = smoothedFps > 0.0f ? 1000.0f / smoothedFps : 0.0f;

    if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("FPS: %.0f", smoothedFps);
        ImGui::Text("Frame time: %.0f ms", frameTimeMs);
        ImGui::Text("Mouse position: (%d, %d)", input.mouseX, input.mouseY);
        ImGui::TextUnformatted(debug.currentStateInfo.c_str());
    }

    if (ImGui::CollapsingHeader("Game Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SeparatorText("Stage / Wave");
        ImGui::InputInt("Stage", &debug.game.stage);
        ImGui::InputInt("Wave", &debug.game.wave);
        if (ImGui::Button("Reload Stage/Wave")) {
            debug.game.isStageWaveReloadRequested = true;
        }

        ImGui::SeparatorText("Collision");
        ImGui::Checkbox("Show Hitboxes", &debug.game.showHitboxes);
    }

    if (ImGui::CollapsingHeader("Entities", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char *preview = "Select entity";

        if (debug.game.selectedEntity.has_value()) {
            static std::string previewStr;
            previewStr = "Entity " + std::to_string(debug.game.selectedEntity.value());
            preview = previewStr.c_str();
        }

        if (ImGui::BeginCombo("Entity", preview)) {
            for (const auto &entity : debug.game.entities) {
                bool isSelected = debug.game.selectedEntity.has_value() && debug.game.selectedEntity.value() == entity;

                std::string label = "Entity " + std::to_string(entity);

                if (ImGui::Selectable(label.c_str(), isSelected)) {
                    debug.game.selectedEntity = entity;
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