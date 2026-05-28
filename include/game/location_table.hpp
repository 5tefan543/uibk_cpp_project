#pragma once
#include "game/ecs/components/position.hpp"
#include "game/ecs/entity.hpp"
#include "game/ecs/registry.hpp"
#include "utils/vector.hpp"
#include <unordered_set>

namespace game {

// Lookup 2D Table for finding entities based on their location in the world grid with dimensions view::gridWidth X
// view::gridHeight.
struct LocationTable {
    const Vec2<unsigned> numBuckets;
    const Vec2<float> bucketSize;

    LocationTable(const Vec2<unsigned> numBuckets);
    void update(const Registry &registry);
    std::unordered_set<Entity> getEntitiesNear(const Vec2<float> position, const float radius);
    std::vector<std::tuple<Entity, Position>> getEntitiesInRange(const Vec2<float> position, const float radius,
                                                                 const Registry &registry);
    const std::vector<Entity> *cgetBucket(const unsigned buckIx, const unsigned buckIy) const
    {
        return bucketGrid_[buckIy * numBuckets.x + buckIx].get();
    }
    const std::vector<std::unique_ptr<std::vector<Entity>>> &cgetGrid() { return bucketGrid_; }

  private:
    std::vector<std::unique_ptr<std::vector<Entity>>> bucketGrid_;
    std::vector<Entity> *getBucket(const unsigned buckIx, const unsigned buckIy)
    {
        return bucketGrid_[buckIy * numBuckets.x + buckIx].get();
    }
    std::tuple<Vec2<unsigned>, Vec2<unsigned>> getBucketIndices(Vec2<float> startGrid, Vec2<float> stopGrid);
};

} // namespace game