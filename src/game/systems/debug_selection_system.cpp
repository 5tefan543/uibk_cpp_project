#include "game/ecs/systems/debug_selection_system.hpp"
#include "game/ecs/components/map.hpp"
#include "game/ecs/components/sprite.hpp"

namespace game {

void DebugSelectionSystem::update(Registry &registry, const controller::InputState &input, bool isDebugActive,
                                  game::GameDebugSession &debugSession)
{
    if (!isDebugActive) {
        return;
    }

    auto isEntitySelected = [&debugSession](Entity entity) {
        return debugSession.selectedEntity.has_value() && debugSession.selectedEntity.value() == entity;
    };

    for (auto entity : registry.view<Map>()) {
        Map &map = registry.getComponent<Map>(entity);
        map.isSelected = isEntitySelected(entity);
    }

    for (auto entity : registry.view<Sprite>()) {
        Sprite &sprite = registry.getComponent<Sprite>(entity);
        sprite.isSelected = isEntitySelected(entity);
    }
}

} // namespace game