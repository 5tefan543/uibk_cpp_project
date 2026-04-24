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

void DebugUI::render(controller::DebugContext &debug, const controller::InputState &input, float fps)
{
    if (!debug.active) {
        return;
    }

    // Set the next window position before calling Begin()
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);
    ImGui::Begin("Debug", &debug.active, ImGuiWindowFlags_AlwaysAutoResize);

    renderStats(fps, input, debug);
    renderGameSettings(debug);
    renderGameSession(debug);

    ImGui::End();
}

void DebugUI::renderStats(float fps, const controller::InputState &input, controller::DebugContext &debug)
{
    float smoothedFps = (prevFps_ > 0.0f) ? 0.99f * prevFps_ + 0.01f * fps : fps;
    prevFps_ = smoothedFps;
    float frameTimeMs = smoothedFps > 0.0f ? 1000.0f / smoothedFps : 0.0f;

    if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("FPS: %.0f", smoothedFps);
        ImGui::Text("Frame time: %.0f ms", frameTimeMs);
        ImGui::Text("Mouse position: (%d, %d)", input.mouseX, input.mouseY);
        ImGui::TextUnformatted(debug.currentStateInfo.c_str());
    }
}

void DebugUI::renderGameSettings(controller::DebugContext &debug)
{
    if (ImGui::CollapsingHeader("Game Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SeparatorText("Stage / Wave");
        ImGui::InputInt("Stage", &debug.gameSettings.stage);
        ImGui::InputInt("Wave", &debug.gameSettings.wave);
        if (ImGui::Button("Reload Stage/Wave") && debug.gameSession) {
            debug.gameSession->isStageWaveReloadRequested = true;
        }

        ImGui::SeparatorText("Collision");
        ImGui::Checkbox("Show Hitboxes", &debug.gameSettings.showHitboxes);
    }
}

void DebugUI::renderGameSession(controller::DebugContext &debug)
{
    if (debug.gameSession) {
        game::GameDebugSession &gameSession = *debug.gameSession;

        if (ImGui::CollapsingHeader("Game Session", ImGuiTreeNodeFlags_DefaultOpen)) {

            if (ImGui::Button("Open Store")) {
                debug.gameSession->isStoreOpenRequested = true;
            }

            if (ImGui::Button("Destroy Player")) {
                debug.gameSession->isPlayerDestructionRequested = true;
            }

            std::string updateSystemsLabel =
                gameSession.isSystemUpdateActive ? "Pause Simulation" : "Resume Simulation";
            if (ImGui::Button(updateSystemsLabel.c_str())) {
                debug.gameSession->isSystemUpdateActive = !debug.gameSession->isSystemUpdateActive;
            }

            renderEcsManagement(gameSession);
        }
    }
}

void DebugUI::renderEcsManagement(game::GameDebugSession &gameSession)
{
    if (ImGui::CollapsingHeader("ECS", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::Text("Entity count: %zu", gameSession.registry.entities().size());

        if (ImGui::BeginListBox("##entity_list")) {
            for (const auto &entity : gameSession.registry.entities()) {
                const bool isSelected =
                    gameSession.selectedEntity.has_value() && gameSession.selectedEntity.value() == entity;

                std::string entityLabel = "Entity " + std::to_string(entity);

                if (ImGui::Selectable(entityLabel.c_str(), isSelected)) {
                    gameSession.selectedEntity = entity;
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndListBox();
        }

        if (gameSession.selectedEntity.has_value()) {
            game::Entity entity = gameSession.selectedEntity.value();

            if (gameSession.registry.isEntityAlive(entity)) {
                ImGui::Text("Selected entity: %u", entity);

                if (gameSession.registry.hasComponent<game::Position>(entity)) {
                    renderComponent(gameSession.registry.getComponent<game::Position>(entity));
                }
                if (gameSession.registry.hasComponent<game::Velocity>(entity)) {
                    renderComponent(gameSession.registry.getComponent<game::Velocity>(entity));
                }
                if (gameSession.registry.hasComponent<game::PlayerTag>(entity)) {
                    renderComponent(gameSession.registry.getComponent<game::PlayerTag>(entity));
                }
                if (gameSession.registry.hasComponent<game::Sprite>(entity)) {
                    renderComponent(gameSession.registry.getComponent<game::Sprite>(entity));
                }
                if (gameSession.registry.hasComponent<game::Map>(entity)) {
                    renderComponent(gameSession.registry.getComponent<game::Map>(entity));
                }
                if (gameSession.registry.hasComponent<game::Camera>(entity)) {
                    renderComponent(gameSession.registry.getComponent<game::Camera>(entity));
                }

            } else {
                gameSession.selectedEntity.reset();
            }
        } else {
            ImGui::Text("No entity selected");
        }
    }
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

void DebugUI::renderComponent(game::Sprite &c)
{
    ImGui::SeparatorText("Sprite");
    ImGui::InputInt("currentFrame", &c.currentFrame);
    ImGui::InputInt("totalFrames", &c.totalFrames);
    ImGui::InputFloat("frameDuration", &c.frameDuration);
    ImGui::InputFloat("frameTimer", &c.frameTimer);
}

void DebugUI::renderComponent(game::Map &c)
{
    ImGui::SeparatorText("Map");
    ImGui::InputFloat("width", &c.width);
    ImGui::InputFloat("height", &c.height);
    ImGui::InputFloat("x", &c.x);
    ImGui::InputFloat("y", &c.y);
}

void DebugUI::renderComponent(game::Camera &c)
{
    ImGui::SeparatorText("Camera");
    ImGui::InputFloat("x", &c.x);
    ImGui::InputFloat("y", &c.y);
    ImGui::InputFloat("viewportHeight", &c.viewportHeight);
    ImGui::InputFloat("viewportWidth", &c.viewportWidth);
}
} // namespace ui