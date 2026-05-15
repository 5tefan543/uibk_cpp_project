#include "game/ecs/systems/debug_selection_system.hpp"
#include "game/ecs/components/camera.hpp"
#include "game/ecs/components/map.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sprite.hpp"

namespace game {

void DebugSelectionSystem::update(Registry &registry, const controller::InputState &input, bool isDebugActive,
                                  GameDebugSession &debugSession)
{
    if (!isDebugActive) {
        clearSelection(registry, debugSession);
        return;
    }

    if (input.controlHeld && input.mouseLeftPressed) {
        debugSession.selectedEntity = getEntityAtMousePosition(registry, input);
    }

    updateSelection(registry, debugSession);
}

void DebugSelectionSystem::clearSelection(Registry &registry, GameDebugSession &debugSession)
{
    if (!debugSession.selectedEntity.has_value()) {
        return;
    }

    Entity selectedEntity = debugSession.selectedEntity.value();

    if (registry.hasComponent<Map>(selectedEntity)) {
        registry.getComponent<Map>(selectedEntity).isSelected = false;
    }

    if (registry.hasComponent<Sprite>(selectedEntity)) {
        registry.getComponent<Sprite>(selectedEntity).isSelected = false;
    }
}

bool contains(float mouseX, float mouseY, float x, float y, float width, float height)
{
    return mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height;
}

std::optional<Entity> DebugSelectionSystem::getEntityAtMousePosition(Registry &registry,
                                                                     const controller::InputState &input)
{
    for (auto entity : registry.view<Position, Sprite>()) {
        const Position &position = registry.getComponent<Position>(entity);
        const Sprite &sprite = registry.getComponent<Sprite>(entity);

        if (contains(input.mouseGridX, input.mouseGridY, position.x, position.y, sprite.width, sprite.height)) {
            return entity;
        }
    }

    for (auto entity : registry.view<Map>()) {
        const Map &map = registry.getComponent<Map>(entity);

        if (contains(input.mouseGridX, input.mouseGridY, map.x, map.y, map.width, map.height)) {
            return entity;
        }
    }

    return std::nullopt;
}

void DebugSelectionSystem::updateSelection(Registry &registry, GameDebugSession &debugSession)
{
    auto isSelected = [&debugSession](Entity entity) {
        return debugSession.selectedEntity.has_value() && debugSession.selectedEntity.value() == entity;
    };

    for (auto entity : registry.view<Map>()) {
        registry.getComponent<Map>(entity).isSelected = isSelected(entity);
    }

    for (auto entity : registry.view<Sprite>()) {
        registry.getComponent<Sprite>(entity).isSelected = isSelected(entity);
    }
}

} // namespace game