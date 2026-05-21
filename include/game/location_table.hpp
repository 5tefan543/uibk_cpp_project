#pragma once
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sprite.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/entity.hpp"
#include "game/ecs/registry.hpp"
#include "view/grid.hpp"
#include <unordered_set>

namespace game {

// Lookup 2D Table for finding entities based on their location in the world grid with dimensions view::gridWidth X
// view::gridHeight.
struct LocationTable {
    void update(const Registry &registry);
    std::unordered_set<Entity> getEntitiesNear(const float x, const float y, float radius);
    LocationTable(unsigned numBuckW, unsigned numBuckH);

  private:
    const unsigned buckWidth;
    const unsigned buckHeight;
    const unsigned numBuckW;
    const unsigned numBuckH;
    std::vector<std::vector<Entity>> bucketGrid_;
    const std::vector<Entity> &cgetBucket(unsigned buckIx, unsigned buckIy)
    {
        return bucketGrid_[buckIy * numBuckW + buckIx];
    }
    std::vector<Entity> &getBucket(unsigned buckIx, unsigned buckIy) { return bucketGrid_[buckIy * numBuckW + buckIx]; }
};

} // namespace game