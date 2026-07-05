#include "game/ecs/systems/sound_system.hpp"
#include "game/ecs/components/sound.hpp"
#include "game/ecs/entity.hpp"
#include <vector>

namespace game {

void SoundSystem::update(Registry &registry)
{
    auto entities = registry.view<Sound>();
    for (Entity entity : entities) {
        Sound &sound = registry.getComponent<Sound>(entity);
        manager_.playSound(sound.file);
        registry.removeComponent<Sound>(entity);
    }
};
} // namespace game
