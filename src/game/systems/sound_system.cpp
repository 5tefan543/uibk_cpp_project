#include "game/ecs/systems/sound_system.hpp"
#include "game/ecs/components/sound.hpp"
#include "game/ecs/entity.hpp"
#include <vector>

namespace game {

void SoundSystem::update(Registry &registry)
{
    auto entities = registry.view<SoundComponent>();
    for (Entity entity : entities) {
        SoundComponent &sound = registry.getComponent<SoundComponent>(entity);
        manager_.playSound(sound.name);
        registry.removeComponent<SoundComponent>(entity);
    }
};
} // namespace game
