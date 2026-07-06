#pragma once
#include "game/ecs/components/position.hpp"
#include "game/ecs/entity.hpp"
#include "game/ecs/registry.hpp"
#include "geometry/vector.hpp"
#include <unordered_set>

namespace game {

// Lookup 2D Table for finding entities based on their location in the world grid
class LocationTable {
  private:
    std::vector<std::vector<Entity>> bucketGrid_;
    geometry::Vec2<float> largestHitboxDims_ = {0, 0};

    std::vector<Entity> &getBucket(const unsigned buckIx, const unsigned buckIy);
    std::tuple<geometry::Vec2<unsigned>, geometry::Vec2<unsigned>>
    getBucketIndices(geometry::Vec2<float> startGrid, geometry::Vec2<float> stopGrid) const;

  public:
    const geometry::Vec2<unsigned> numBuckets;
    const geometry::Vec2<float> bucketSize;

    LocationTable(const geometry::Vec2<unsigned> numBuckets, const geometry::Vec2<float> totalGridSize);
    void update(const Registry &registry);
    std::unordered_set<Entity> getEnemiesNear(const geometry::Vec2<float> position, const float radius) const;
    std::vector<std::tuple<Entity, Position>> getEnemiesInRange(const geometry::Vec2<float> position,
                                                                const float radius, const Registry &registry) const;
    const std::vector<Entity> &cgetBucket(const unsigned buckIx, const unsigned buckIy) const;
    const std::vector<std::vector<Entity>> &cgetGrid();
};

// Inline definitions suggesting the compiler to inline their operations since they are called in the innermost loops of
// the location table's retrival and update methods.

// -
inline const std::vector<Entity> &LocationTable::cgetBucket(const unsigned buckIx, const unsigned buckIy) const
{
    return bucketGrid_[buckIy * numBuckets.x + buckIx];
}

inline std::vector<Entity> &LocationTable::getBucket(const unsigned buckIx, const unsigned buckIy)
{
    return bucketGrid_[buckIy * numBuckets.x + buckIx];
}

inline const std::vector<std::vector<Entity>> &LocationTable::cgetGrid()
{
    return bucketGrid_;
}

} // namespace game