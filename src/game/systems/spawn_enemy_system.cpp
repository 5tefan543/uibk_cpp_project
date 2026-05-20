#include "game/ecs/systems/spawn_enemy_system.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"
#include "view/sprite.hpp"
#include <random>

namespace game {

void SpawnEnemySystem::update(Registry &registry)
{
    // delete all existing enemies
    for (Entity enemy : registry.view<EnemyTag>()) {
        registry.destroyEntity(enemy);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> posDist(200.0f, 800.0f);
    std::uniform_real_distribution<> velDist(0.0f, 0.0f);

    // Spawn 3 enemies at different positions
    for (int i = 0; i < 3; ++i) {
        Entity enemy = registry.createEntity();
        registry.addComponent<EnemyTag>(enemy, {});
        registry.addComponent<Position>(enemy, {static_cast<float>(posDist(gen)), static_cast<float>(posDist(gen))});
        registry.addComponent<Velocity>(enemy, {static_cast<float>(velDist(gen)), static_cast<float>(velDist(gen))});

        Animation animation = {
            .baseTexturePath = "assets/characters/enemy_1_",
        };
        registry.addComponent<Animation>(enemy, animation);
        registry.addComponent<view::Sprite>(enemy, {.imagePath = animation.baseTexturePath + "right_1.png"});
    }
}

} // namespace game