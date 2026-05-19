#pragma once
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

template <unsigned buckWidth, unsigned buckHeight, unsigned gridWidth, unsigned gridHeight>
struct LocationTable {
    void update(const Registry &registry);
    std::unordered_set<Entity> getEntitiesNear(const float x, const float y, float radius);
    LocationTable();

  private:
    const unsigned numBuckW = gridWidth / buckWidth;
    const unsigned numBuckH = gridHeight / buckHeight;
    std::unique_ptr<std::vector<Entity>> bucketGrid_[gridHeight / buckHeight][gridWidth / buckWidth];
};

using locTab = LocationTable<40, 40, (unsigned)view::gridWidth, (unsigned)view::gridHeight>;

template <unsigned buckWidth, unsigned buckHeight, unsigned gridWidth, unsigned gridHeight>
LocationTable<buckWidth, buckHeight, gridWidth, gridHeight>::LocationTable()
{
    for (unsigned h = 0; h < numBuckH; h++) {
        for (unsigned w = 0; w < numBuckW; w++) {
            bucketGrid_[h][w] = std::make_unique<std::vector<Entity>>();
        }
    }
}

template <unsigned buckWidth, unsigned buckHeight, unsigned gridWidth, unsigned gridHeight>
void LocationTable<buckWidth, buckHeight, gridWidth, gridHeight>::update(const Registry &registry)
{
    // bool cleanup = false;
    auto enemies = registry.view<Sprite, Position, Velocity, EnemyTag>();
    for (auto enemy : enemies) {
        const Position &position = registry.getComponent<Position>(enemy);
        const Sprite &sprite = registry.getComponent<Sprite>(enemy);

        // Put entity in every bucket its sprite overlaps with.
        // TODO: we probably want to use hitbox instead of sprite dimensions
        // Distances can be determined either way since hitbox & sprite have the same center.

        // min/max(): better save than sorry - making no assumtions of logic positioning entities/sprites
        unsigned buckIx = (unsigned)std::max(0.0f, (position.x / (float)gridWidth));
        unsigned buckIy = (unsigned)std::max(0.0f, (position.y / (float)gridHeight));
        bucketGrid_[buckIx][buckIy]->emplace_back(enemy);

        for (float y = position.y + buckHeight; y <= (position.y + sprite.height); y += buckHeight) {
            for (float x = position.x; x <= (position.x + sprite.width); x += buckWidth) {
                buckIy = std::min((unsigned)(y / buckHeight), numBuckH - 1);
                buckIx = std::min((unsigned)(x / buckWidth), numBuckW - 1);
                bucketGrid_[buckIy][buckIx]->emplace_back(enemy);
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
template <unsigned buckWidth, unsigned buckHeight, unsigned gridWidth, unsigned gridHeight>
std::unordered_set<Entity>
LocationTable<buckWidth, buckHeight, gridWidth, gridHeight>::getEntitiesNear(const float x, const float y, float radius)
{
    // min/max(): better save than sorry - making no assumtions of logic positioning entities/sprites
    std::unordered_set<Entity> inRange;
    unsigned buckIx = (unsigned)(std::max(0.0f, x - radius) / (float)buckWidth);
    unsigned buckIy = (unsigned)(std::max(0.0f, y - radius) / (float)buckHeight);
    for (; (float)buckIy <= (y + radius); buckIy += gridHeight) {
        for (; (float)buckIx <= (x + radius); buckIx += gridWidth) {
            // unordered_set removes duplicate entities introduced by update
            inRange.insert_range(*bucketGrid_[std::min(buckIy, numBuckH - 1)][std::min(buckIx, numBuckW - 1)].get());
        }
    }
    return inRange;
}

} // namespace game