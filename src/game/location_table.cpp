#include "game/location_table.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sprite.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/entity.hpp"
#include "game/ecs/registry.hpp"
#include "view/grid.hpp"
#include <iostream>
#include <unordered_set>

namespace game {

LocationTable::LocationTable(const unsigned buckWidth, const unsigned buckHeight)
    : buckWidth(buckWidth), buckHeight(buckHeight), numBuckW(view::gridWidth / buckWidth),
      numBuckH(view::gridHeight / buckHeight)
{
    bucketGrid_ = std::vector<std::unique_ptr<std::vector<Entity>>>();
    bucketGrid_.reserve(numBuckW * numBuckH);
    for (size_t i = 0; i < numBuckW * numBuckH; i++) {
        bucketGrid_.emplace_back(std::make_unique<std::vector<Entity>>());
    }
}

void LocationTable::update(const Registry &registry)
{
    for (auto &bucket : bucketGrid_) {
        bucket->clear(); // Leaves the capacity() of the vector unchanged
    }

    // bool cleanup = false;
    auto entities = registry.view<Sprite, Position, Velocity, EnemyTag>();
    for (auto entity : entities) {
        const Position &position = registry.getComponent<Position>(entity);
        const Sprite &sprite = registry.getComponent<Sprite>(entity);

        // Put entity in every bucket its sprite overlaps with.
        // TODO: we probably want to use hitbox instead of sprite dimensions
        // Distances can be determined either way since hitbox & sprite have the same center.

        // min/max(): better save than sorry - making no assumtions of logic positioning entities/sprites
        const float startX = (unsigned)std::max(0.0f, (position.x / buckWidth));
        const float startY = (unsigned)std::max(0.0f, (position.y / buckHeight));

        const unsigned numX = std::min(
            (unsigned)((position.x + sprite.width) / buckWidth) - (unsigned)(position.x / buckWidth), numBuckW - 1);
        const unsigned numY = std::min(
            (unsigned)((position.y + sprite.height) / buckHeight) - (unsigned)(position.y / buckHeight), numBuckH - 1);

        for (unsigned buckY = startY; buckY <= numY + startY; buckY++) {
            for (unsigned buckX = startX; buckX <= numX + startX; buckX++) {
                getBucket(buckX, buckY)->emplace_back(entity);
            }
        }

        // ### Not-thought-through idea how to figure out if we want to free up memory.
        // ### For one wave worst case with no cleanup and N entities max:
        // ### numBuckets := (gridWidth / buckGrdWidth) * (gridHeight / buckGrdHeight)
        // ### -> numBuckets * N * sizeof(Entity) Bytes
        // ### -> numBuckets * N * 4 Bytes
        // ### -> (1920 / 30) * (1080 / 30) * 4 Bytes * 10 000 = 92.16MB
        // ### ...which seems to be ok?
        // ### Another aspect: the overall amount of entities per waves/stages will never decrease with new
        // ### levels/stages so in the long run there will always be a possible situation for any bucket where
        // the
        // ###bucket needs more memory.
        // const bool tooUnused =
        //     (bucket->capacity() - bucket->size()) > (entities.size() / (buckGrdWidth * buckGrdHeight * 1.3));
        // const bool concentrated = bucket->size() >= entities.size() / 2;
        // if (tooUnused || concentrated) {
        //     cleanup = true;
        // }
    }

    // ### Free memory - shrink bucket sizes
    // if (cleanup) {
    //     for (unsigned h = 0; h < buckGrdHeight; h++) {
    //         for (unsigned w = 0; w < buckGrdWidth; w++) {
    //             bucketGrid_[h][w]->shrink_to_fit();
    //         }
    //     }
    // }
}

// Guarantees to return all entities whose hitbox/sprite is inside the radius but there might be some included that are
// not. Extra filtering is required if strictly those inside the radius are required.
std::unordered_set<Entity> LocationTable::getEntitiesNear(const float x, const float y, const float radius)
{
    // min/max(): better save than sorry - making no assumtions of logic positioning entities/sprites
    std::unordered_set<Entity> inRange;
    unsigned buckIx = (unsigned)(std::max(0.0f, x - radius) / (float)buckWidth);
    unsigned buckIy = (unsigned)(std::max(0.0f, y - radius) / (float)buckHeight);
    for (unsigned y = buckIy; (float)y <= ((float)buckIy + radius); y += view::gridHeight) {
        for (unsigned x = buckIx; (float)x <= ((float)buckIx + radius); x += view::gridWidth) {
            // unordered_set removes duplicate entities introduced by update (sprite crossing bucket borders)
            inRange.insert_range(*cgetBucket(std::min(x, numBuckW - 1), std::min(y, numBuckH - 1)));
        }
    }
    return inRange;
}

} // namespace game