#pragma once

#include "game/ecs/registry.hpp"

namespace game {

struct LocationTable {
    void update(const Registry &registry);
    std::vector<Entity> getEntitiesNear(const float x, const float y);

  private:
    static const unsigned buckGrdWidth = 40;  // 1920 / 40 = 48
    static const unsigned buckGrdHeight = 40; // 1080 / 24 = 45
    std::unique_ptr<std::vector<Entity>> bucketGrid_[buckGrdHeight][buckGrdWidth];
};

} // namespace game