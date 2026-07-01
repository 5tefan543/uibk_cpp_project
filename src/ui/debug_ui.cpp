#include "ui/debug_ui.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "logging/log.hpp"
#include <imgui.h>
#include <vector>

namespace ui {

DebugUI::DebugUI()
{
    logger::log(logger::DEBUG, "DebugUI constructed");
}

DebugUI::~DebugUI()
{
    logger::log(logger::DEBUG, "DebugUI destructed");
}

void DebugUI::render(const controller::InputState &input, const controller::timeDelta &dt)
{
    controller::DebugContext &debug = controller::DebugContext::get();
    if (!debug.active) {
        return;
    }

    // Set the next window position before calling Begin()
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);
    ImGui::Begin("Debug", &debug.active, ImGuiWindowFlags_AlwaysAutoResize);

    renderStats(dtSec, input, debug);
    renderGameSettings(debug);
    renderGameSession(debug);

    ImGui::End();
}

void DebugUI::renderStats(const controller::timeDelta &dt, const controller::InputState &input,
                          controller::DebugContext &debug)
{
    float frameTimeMs = std::chrono::duration<double, std::milli>(dt).count();
    float fps = 1.0f / frameTimeMs * (double)1e3;
    float smoothedFps = (prevFps_ > 0.0f) ? 0.99f * prevFps_ + 0.01f * fps : fps;
    prevFps_ = smoothedFps;
    frameTimeMs = smoothedFps > 0.0f ? 1000.0f / smoothedFps : 0.0f;

    if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("FPS: %.0f", smoothedFps);
        ImGui::Text("Frame time: %.0f ms", frameTimeMs);
        ImGui::Text("Mouse position: (%f, %f)", input.mouseGrid.x, input.mouseGrid.y);
        ImGui::TextUnformatted(debug.currentStateInfo.c_str());
    }
}

void DebugUI::renderGameSettings(controller::DebugContext &debug)
{
    if (ImGui::CollapsingHeader("Game Settings", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::SeparatorText("Collision");
        ImGui::Checkbox("Show Hitboxes", &debug.gameSettings.showHitboxes);
        ImGui::Checkbox("Show Location Table", &debug.gameSettings.showLocationTable);
    }
}

void DebugUI::renderGameSession(controller::DebugContext &debug)
{
    if (debug.gameSession) {
        game::GameDebugSession &gameSession = *debug.gameSession;

        if (ImGui::CollapsingHeader("Game Session", ImGuiTreeNodeFlags_DefaultOpen)) {

            int wavesPerStage = controller::PersistenceManager::getConfig().wavesPerStage;

            ImGui::SeparatorText("Stage / Wave");
            const bool didStageChange = ImGui::InputInt("Stage", &gameSession.stage);
            const bool didWaveChange = ImGui::InputInt("Wave", &gameSession.wave);

            // keep values >= 1
            gameSession.stage = std::max(1, gameSession.stage);
            gameSession.wave = std::max(1, gameSession.wave);

            if (didStageChange) {
                // move to first wave of the selected stage
                gameSession.wave = (gameSession.stage - 1) * wavesPerStage + 1;
            }

            if (didWaveChange) {
                // derive stage from wave
                gameSession.stage = ((gameSession.wave - 1) / wavesPerStage) + 1;
            }

            if (ImGui::Button("Reload Stage/Wave") && debug.gameSession) {
                debug.gameSession->isStageWaveReloadRequested = true;
            }

            ImGui::Checkbox("Pause Clock", &debug.gameSession->isClockPaused);

            ImGui::SeparatorText("Persistence Management");
            if (ImGui::Button("Save Game")) {
                debug.gameSession->isSaveGameRequested = true;
            }
            if (controller::PersistenceManager::hasSavedGame()) {
                if (ImGui::Button("Delete Saved Game")) {
                    controller::PersistenceManager::deleteSave();
                }
            }

            ImGui::SeparatorText("Progression Store");
            if (ImGui::Button("Open Store")) {
                debug.gameSession->isStoreOpenRequested = true;
            }

            renderEcsManagement(debug, gameSession);
        }
    }
}

void DebugUI::renderEcsManagement(controller::DebugContext &debug, game::GameDebugSession &gameSession)
{
    if (ImGui::CollapsingHeader("ECS", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::SeparatorText("Game Control");
        if (ImGui::Button("Destroy Player")) {
            debug.gameSession->isPlayerDestructionRequested = true;
        }

        std::string updateSystemsLabel = gameSession.isSystemUpdateActive ? "Pause Simulation" : "Resume Simulation";
        if (ImGui::Button(updateSystemsLabel.c_str())) {
            debug.gameSession->isSystemUpdateActive = !debug.gameSession->isSystemUpdateActive;
            logger::log(logger::DEBUG, "pause game!");
        }

        ImGui::SeparatorText("Entity Management");
        ImGui::Text("Entity count: %zu", gameSession.registry.entities().size());
        ImGui::Text("Select on screen: Ctrl + Left-Mouse-Btn");

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

                if (gameSession.registry.hasComponent<game::CameraTag>(entity)) {
                    renderComponent(gameSession.registry.getComponent<game::CameraTag>(entity));
                }
                if (gameSession.registry.hasComponent<game::Position>(entity)) {
                    renderComponent(gameSession.registry.getComponent<game::Position>(entity));
                }
                if (gameSession.registry.hasComponent<game::Velocity>(entity)) {
                    renderComponent(gameSession.registry.getComponent<game::Velocity>(entity));
                }
                if (gameSession.registry.hasComponent<game::PlayerStats>(entity)) {
                    renderComponent(gameSession.registry.getComponent<game::PlayerStats>(entity));
                }
                if (gameSession.registry.hasComponent<game::EnemyStats>(entity)) {
                    renderComponent(gameSession.registry.getComponent<game::EnemyStats>(entity));
                }
                if (gameSession.registry.hasComponent<game::Animation>(entity)) {
                    renderComponent(gameSession.registry.getComponent<game::Animation>(entity));
                }
                if (gameSession.registry.hasComponent<view::Sprite>(entity)) {
                    renderComponent(gameSession.registry.getComponent<view::Sprite>(entity));
                }
                if (gameSession.registry.hasComponent<game::HitBox>(entity)) {
                    if (gameSession.registry.hasComponent<view::Sprite>(entity)) {
                        renderComponent(gameSession.registry.getComponent<game::HitBox>(entity),
                                        gameSession.registry.getComponent<view::Sprite>(entity).imagePath);
                    } else {
                        renderComponent(gameSession.registry.getComponent<game::HitBox>(entity));
                    }
                }
            } else {
                gameSession.selectedEntity.reset();
            }
        } else {
            ImGui::Text("No entity selected");
        }
    }
}

void DebugUI::renderComponent(game::CameraTag &c)
{
    ImGui::PushID("CameraComponent");

    ImGui::SeparatorText("CameraTag");
    ImGui::InputFloat("margin", &c.margin);

    ImGui::PopID();
}

void DebugUI::renderComponent(game::Stats &c)
{
    if (ImGui::CollapsingHeader("Stats")) {

        ImGui::PushID("StatsComponent");

        ImGui::InputFloat("maxHealth", &c.maxHealth);
        ImGui::InputFloat("health", &c.health);
        ImGui::InputFloat("attackPower", &c.attackPower);
        ImGui::InputFloat("attackSpeed", &c.attackSpeed);
        ImGui::InputFloat("speedOfAttack", &c.speedOfAttack);
        ImGui::InputFloat("attackRange", &c.attackRange);
        ImGui::InputFloat("defense", &c.defense);
        ImGui::InputFloat("moveSpeed", &c.moveSpeed);

        ImGui::PopID();
    }
}

void DebugUI::renderComponent(game::PlayerStats &c)
{
    renderComponent(static_cast<game::Stats &>(c));

    ImGui::PushID("PlayerStatsComponent");

    ImGui::SeparatorText("PlayerStats");
    ImGui::Checkbox("hasDash", &c.hasDash);

    ImGui::PopID();
}

void DebugUI::renderComponent(game::EnemyStats &c)
{
    renderComponent(static_cast<game::Stats &>(c));

    ImGui::PushID("EnemyStatsComponent");

    ImGui::SeparatorText("EnemyStats");
    ImGui::InputInt("scoreReward", &c.scoreReward);

    ImGui::PopID();
}

void DebugUI::renderComponent(game::Position &c)
{
    ImGui::PushID("PositionComponent");

    ImGui::SeparatorText("Position");
    ImGui::InputFloat("x", &c.p.x);
    ImGui::InputFloat("y", &c.p.y);

    ImGui::PopID();
}

void DebugUI::renderComponent(game::Velocity &c)
{
    ImGui::PushID("VelocityComponent");

    ImGui::SeparatorText("Velocity");
    ImGui::InputFloat("dx", &c.v.x);
    ImGui::InputFloat("dy", &c.v.y);

    ImGui::PopID();
}

void DebugUI::renderComponent(game::Animation &c)
{
    if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::PushID("AnimationComponent");

        ImGui::InputFloat("frameTimer", &c.frameTimer);
        ImGui::InputFloat("stateTimeRemaining", &c.stateTimeRemaining);
        ImGui::Text("state: %s", toString(c.state));
        ImGui::Text("direction: %s", toString(c.direction));

        ImGui::PopID();
    }
}

void DebugUI::renderComponent(view::Sprite &c)
{
    if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushID("SpriteComponent");

        ImGui::SeparatorText("Sprite");
        ImGui::InputFloat("width", &c.rect.size.x);
        ImGui::InputFloat("height", &c.rect.size.y);
        ImGui::Text("imagePath: %s", c.imagePath.c_str());

        ImGui::PopID();
    }
}

void DebugUI::renderComponent(game::HitBox &c)
{
    if (ImGui::CollapsingHeader("HitBox", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::PushID("HitboxComponent");

        ImGui::InputFloat("width", &c.size.x);
        ImGui::InputFloat("height", &c.size.y);

        ImGui::InputFloat("offset x", &c.offset.x);
        ImGui::InputFloat("offset y", &c.offset.y);
        ImGui::PopID();
    }
}

void DebugUI::renderComponent(game::HitBox &c, const std::string &texturePath)
{

    if (ImGui::CollapsingHeader("HitBox", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::PushID("Hitbox");

        ImGui::InputFloat("width", &c.size.x);
        ImGui::InputFloat("height", &c.size.y);

        ImGui::InputFloat("offset x", &c.offset.x);
        ImGui::InputFloat("offset y", &c.offset.y);

        auto searchAnimConfig = [&](const config::AnimationConfig &animCfg) -> const config::HitBoxConfig * {
            for (const auto &[state, stateCfg] : animCfg.stateToStateConfig) {
                for (const auto &[dir, frames] : stateCfg.directionToFrames) {
                    for (const auto &spriteCfg : frames) {
                        if (spriteCfg.texture.path == texturePath) {
                            return &spriteCfg.hitBox;
                        }
                    }
                }
            }
            return nullptr;
        };

        if (ImGui::Button("Load from Config")) {
            const config::GameConfig &cfg = controller::PersistenceManager::getConfig();
            const config::HitBoxConfig *found = nullptr;

            for (const auto *playerCfg : {&cfg.playerClasses.melee, &cfg.playerClasses.ranged}) {
                if (!found)
                    found = searchAnimConfig(playerCfg->animations);
                if (!found)
                    found = searchAnimConfig(playerCfg->attack.projectile.animations);
                if (!found)
                    found = searchAnimConfig(playerCfg->attack.beam.animations);
                if (!found)
                    found = searchAnimConfig(playerCfg->attack.area.animations);
            }
            for (const auto *enemyCfg : {&cfg.enemyClasses.blob, &cfg.enemyClasses.boss}) {
                if (!found)
                    found = searchAnimConfig(enemyCfg->animations);
                if (!found)
                    found = searchAnimConfig(enemyCfg->attack.projectile.animations);
                if (!found)
                    found = searchAnimConfig(enemyCfg->attack.beam.animations);
                if (!found)
                    found = searchAnimConfig(enemyCfg->attack.area.animations);
            }
            for (const auto &spriteCfg : cfg.mapConfig.mapSprites) {
                if (!found && spriteCfg.texture.path == texturePath) {
                    found = &spriteCfg.hitBox;
                }
            }
            if (!found && cfg.fallbackSprite.texture.path == texturePath) {
                found = &cfg.fallbackSprite.hitBox;
            }

            if (found) {
                c.size = found->size;
                c.offset = found->offset;
                logger::log(logger::DEBUG, "HitBox loaded from config for texture: " + texturePath);
            } else {
                logger::log(logger::DEBUG, "No config entry found for texture: " + texturePath);
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Save to Config")) {
            config::GameConfig cfg = controller::PersistenceManager::getConfig();
            const config::HitBoxConfig hb{c.offset, c.size};
            int updateCount = 0;

            auto countedUpdate = [&](config::AnimationConfig &animCfg) {
                for (auto &[state, stateCfg] : animCfg.stateToStateConfig) {
                    for (auto &[dir, frames] : stateCfg.directionToFrames) {
                        for (auto &spriteCfg : frames) {
                            if (spriteCfg.texture.path == texturePath) {
                                spriteCfg.hitBox = hb;
                                ++updateCount;
                            }
                        }
                    }
                }
            };

            for (auto *playerCfg : {&cfg.playerClasses.melee, &cfg.playerClasses.ranged}) {
                countedUpdate(playerCfg->animations);
                countedUpdate(playerCfg->attack.projectile.animations);
                countedUpdate(playerCfg->attack.beam.animations);
                countedUpdate(playerCfg->attack.area.animations);
            }
            for (auto *enemyCfg : {&cfg.enemyClasses.blob, &cfg.enemyClasses.boss}) {
                countedUpdate(enemyCfg->animations);
                countedUpdate(enemyCfg->attack.projectile.animations);
                countedUpdate(enemyCfg->attack.beam.animations);
                countedUpdate(enemyCfg->attack.area.animations);
            }
            for (auto &spriteCfg : cfg.mapConfig.mapSprites) {
                if (spriteCfg.texture.path == texturePath) {
                    spriteCfg.hitBox = hb;
                    ++updateCount;
                }
            }
            if (cfg.fallbackSprite.texture.path == texturePath) {
                cfg.fallbackSprite.hitBox = hb;
                ++updateCount;
            }

            if (updateCount > 0) {
                if (controller::PersistenceManager::saveConfig(cfg)) {
                    logger::log(logger::DEBUG, "HitBox saved to config for texture: " + texturePath + " ("
                                                   + std::to_string(updateCount) + " entries updated)");
                } else {
                    logger::log(logger::ERROR, "Failed to save config for texture: " + texturePath);
                }
            } else {
                logger::log(logger::DEBUG, "No config entry found to save for texture: " + texturePath);
            }
        }

        ImGui::PopID();
    }
}

} // namespace ui