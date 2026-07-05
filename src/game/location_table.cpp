#include "game/location_table.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/entity.hpp"
#include "game/ecs/registry.hpp"
#include "view/grid.hpp"
#include "view/sprite.hpp"
#include <iostream>
#include <unordered_set>

namespace game {

LocationTable::LocationTable(const geometry::Vec2<unsigned> numBuckets, const geometry::Vec2<float> totalGridSize)
    : numBuckets(numBuckets), bucketSize(totalGridSize / numBuckets.into<float>())
{
    bucketGrid_ = std::vector<std::vector<Entity>>();
    bucketGrid_.reserve(numBuckets.x * numBuckets.y);
    for (size_t i = 0; i < numBuckets.x * numBuckets.y; i++) {
        auto &v = bucketGrid_.emplace_back(std::vector<Entity>());
        v.reserve(50);
    }
}

// Return 2D (x,y) bucket indices [firstBucket, lastBucket]. lastBuck is inclusive -> iterate with "<= lastBuck"
std::tuple<geometry::Vec2<unsigned>, geometry::Vec2<unsigned>>
LocationTable::getBucketIndices(geometry::Vec2<float> startGrid, geometry::Vec2<float> stopGrid) const
{
    using geometry::Vec2;

    const Vec2<float> maxBuckets = (numBuckets - 1).into<float>();
    // clamp(): better save than sorry - making no assumtions of positioning logic of entities/sprites
    Vec2<unsigned> firstBuck = (startGrid / bucketSize).clamp({0, 0}, maxBuckets).into<unsigned>();
    Vec2<unsigned> lastBuck = (stopGrid / bucketSize).clamp({0, 0}, maxBuckets).into<unsigned>();

    return {firstBuck, lastBuck};
}

void LocationTable::update(const Registry &registry)
{
    using geometry::Vec2;

    for (auto &bucket : bucketGrid_) {
        bucket.clear(); // Leaves the capacity() of the vector unchanged
    }

    // bool cleanup = false;
    // auto entities = registry.view<view::Sprite, Position, Velocity, EnemyTag>();
    auto entities = registry.view<Position, HitBox, EnemyTag>();
    for (auto entity : entities) {
        const auto &pos = registry.getComponent<Position>(entity).p;
        // const view::Sprite &sprite = registry.getComponent<view::Sprite>(entity);
        const HitBox &hitbox = registry.getComponent<HitBox>(entity);

        // Put entity in every bucket its sprite overlaps with.
        // TODO: we probably want to use hitbox instead of sprite dimensions
        // Distances can be determined either way since hitbox & sprite have the same center.

        const auto [firstBuck, lastBuck] = getBucketIndices(pos + hitbox.offset, pos + hitbox.offset + hitbox.size);
        // const auto [firstBuck, lastBuck] = getBucketIndices(pos, pos + sprite.rect.size);

        for (unsigned buckY = firstBuck.y; buckY <= lastBuck.y; buckY++) {
            for (unsigned buckX = firstBuck.x; buckX <= lastBuck.x; buckX++) {
                getBucket(buckX, buckY).emplace_back(entity);
            }
        }
        // Not-thought-through idea how to figure out if we want to free up memory.
        // For one wave worst case with no cleanup and N entities max:
        // numBuckets := (gridWidth / buckGrdWidth) * (gridHeight / buckGrdHeight)
        // -> numBuckets * N * sizeof(Entity) Bytes
        // -> numBuckets * N * 4 Bytes
        // -> (1920 / 30) * (1080 / 30) * 4 Bytes * 10 000 = 92.16MB
        // ...which seems to be ok?
        // Another aspect: the overall amount of entities per waves/stages will never decrease with new
        // levels/stages so in the long run there will always be a possible situation for any bucket where
        // the bucket needs more memory.
        //----------------------------------------------------------------------------------------------------
        // const bool tooUnused =
        //     (bucket->capacity() - bucket->size()) > (entities.size() / (buckGrdWidth * buckGrdHeight * 1.3));
        // const bool concentrated = bucket->size() >= entities.size() / 2;
        // if (tooUnused || concentrated) {
        //     cleanup = true;
        // }
    }

    // Free memory - shrink bucket sizes
    // ---------------------------------
    // if (cleanup) {
    //     for (unsigned h = 0; h < buckGrdHeight; h++) {
    //         for (unsigned w = 0; w < buckGrdWidth; w++) {
    //             bucketGrid_[h][w]->shrink_to_fit();
    //         }
    //     }
    // }
}

// Guarantees to return all entities whose hitbox/sprite is inside the radius but there might be some included that are
// outside. Extra filtering is required if strictly those inside the radius are required.
std::unordered_set<Entity> LocationTable::getEntitiesNear(const geometry::Vec2<float> position,
                                                          const float radius) const
{
    std::unordered_set<Entity> inRange;
    const auto [firstBuck, lastBuck] = getBucketIndices(position - radius, position + radius);

    for (unsigned buckY = firstBuck.y; buckY <= lastBuck.y; buckY++) {
        for (unsigned buckX = firstBuck.x; buckX <= lastBuck.x; buckX++) {
            const auto &near = cgetBucket(buckX, buckY);
            inRange.insert(near.cbegin(), near.cend());
        }
    }

    return inRange;
}

// Return Entites being precicesly inside the radius by filtering getEntitiesNear().
std::vector<std::tuple<Entity, Position>> LocationTable::getEntitiesInRange(const geometry::Vec2<float> position,
                                                                            const float radius,
                                                                            const Registry &registry) const
{
    auto entitiesNear = getEntitiesNear(position, radius);
    std::vector<std::tuple<Entity, Position>> inRange;

    for (auto e : entitiesNear) {
        const auto p = registry.getComponent<Position>(e).p;
        if ((position - p).length() <= radius) {
            inRange.emplace_back(std::tuple{e, p});
        }
    }

    return inRange;
}

} // namespace game