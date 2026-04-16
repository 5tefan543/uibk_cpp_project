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

    game::Registry *registry = debug.game.registry;
    std::optional<game::Entity> &selectedEntity = debug.game.selectedEntity;

    if (ImGui::CollapsingHeader("ECS", ImGuiTreeNodeFlags_DefaultOpen)) {

        const std::size_t entityCount = (registry) ? registry->entities().size() : 0;
        ImGui::Text("Entity count: %zu", entityCount);

        if (ImGui::BeginListBox("##entity_list")) {
            if (registry) {
                for (const auto &entity : registry->entities()) {
                    const bool isSelected = selectedEntity.has_value() && selectedEntity.value() == entity;

                    std::string entityLabel = "Entity " + std::to_string(entity);

                    if (ImGui::Selectable(entityLabel.c_str(), isSelected)) {
                        selectedEntity = entity;
                    }

                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
            }

            ImGui::EndListBox();
        }

        if (selectedEntity.has_value()) {
            game::Entity entity = selectedEntity.value();

            if (registry && registry->isEntityAlive(entity)) {
                ImGui::Text("Selected entity: %u", entity);

                if (registry->hasComponent<game::Position>(entity)) {
                    renderComponent(registry->getComponent<game::Position>(entity));
                }
                if (registry->hasComponent<game::Velocity>(entity)) {
                    renderComponent(registry->getComponent<game::Velocity>(entity));
                }
                if (registry->hasComponent<game::PlayerTag>(entity)) {
                    renderComponent(registry->getComponent<game::PlayerTag>(entity));
                }

            } else {
                selectedEntity.reset();
            }
        } else {
            ImGui::Text("No entity selected");
        }
    }

    ImGui::End();
}

void DebugUI::renderComponent(game::PlayerTag &c)
{
    ImGui::SeparatorText("PlayerTag");
    ImGui::InputFloat("moveSpeed", &c.moveSpeed);
}

void DebugUI::renderComponent(game::Position &c)
{
    ImGui::SeparatorText("Position");
    ImGui::InputFloat("x", &c.x);
    ImGui::InputFloat("y", &c.y);
}

void DebugUI::renderComponent(game::Velocity &c)
{
    ImGui::SeparatorText("Velocity");
    ImGui::InputFloat("dx", &c.dx);
    ImGui::InputFloat("dy", &c.dy);
}

} // namespace ui