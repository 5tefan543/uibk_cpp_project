#include "game/ecs/systems/debug_selection_system.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "view/sprite.hpp"

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

    if (registry.hasComponent<view::Sprite>(selectedEntity)) {
        registry.getComponent<view::Sprite>(selectedEntity).isSelected = false;
    }
}

bool contains(float mouseX, float mouseY, float x, float y, float width, float height)
{
    return mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height;
}

std::optional<Entity> DebugSelectionSystem::getEntityAtMousePosition(Registry &registry,
                                                                     const controller::InputState &input)
{
    std::optional<Entity> selectedMap = std::nullopt;

    for (auto entity : registry.view<Position, view::Sprite>()) {
        const Position &position = registry.getComponent<Position>(entity);
        const view::Sprite &sprite = registry.getComponent<view::Sprite>(entity);

        if (contains(input.mouseGridX, input.mouseGridY, position.x, position.y, sprite.width, sprite.height)) {

            if (registry.hasComponent<MapTag>(entity)) {
                selectedMap = entity;
                continue;
            }
            return entity;
        }
    }

    // only return map if no other entity is selected
    if (selectedMap.has_value()) {
        return selectedMap.value();
    }

    return std::nullopt;
}

void DebugSelectionSystem::updateSelection(Registry &registry, GameDebugSession &debugSession)
{
    auto isSelected = [&debugSession](Entity entity) {
        return debugSession.selectedEntity.has_value() && debugSession.selectedEntity.value() == entity;
    };

    for (auto entity : registry.view<view::Sprite>()) {
        registry.getComponent<view::Sprite>(entity).isSelected = isSelected(entity);
    }
}

} // namespace game