#include "game/ecs/systems/health_bar_system.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/health_bar_state.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/stats.hpp"

#include <algorithm>

namespace game {

namespace {

template <typename StatsType>
void updateHealthBarsForTag(Registry &registry, float dt)
{
    for (auto entity : registry.view<StatsType, HealthBarState>()) {
        auto &stats = registry.getComponent<StatsType>(entity);
        auto &bar = registry.getComponent<HealthBarState>(entity);

        // First frame initialisation
        if (bar.previousHealth < 0.0f) {
            bar.previousHealth = stats.health;
            continue;
        }

        if (stats.health < bar.previousHealth) {
            const float dmgNorm = (bar.previousHealth - stats.health) / stats.maxHealth;
            const float currentMissingNorm = 1.0f - (stats.health / stats.maxHealth);
            bar.initialRedBarNorm = std::min(bar.initialRedBarNorm + dmgNorm, currentMissingNorm);
            bar.redBarTimer = HealthBarState::RED_FLASH_DURATION;
        } else if (bar.redBarTimer > 0.0f) {
            bar.redBarTimer = std::max(0.0f, bar.redBarTimer - dt);
            if (bar.redBarTimer == 0.0f) {
                bar.initialRedBarNorm = 0.0f;
            }
        }

        bar.previousHealth = stats.health;
    }
}

} // namespace

void HealthBarSystem::update(Registry &registry, float dt)
{
    updateHealthBarsForTag<PlayerStats>(registry, dt);
    updateHealthBarsForTag<EnemyStats>(registry, dt);
}

} // namespace game
