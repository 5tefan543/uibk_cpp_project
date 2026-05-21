#include "game/location_table.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sprite.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/entity.hpp"
#include "game/ecs/registry.hpp"
#include "view/grid.hpp"
#include <unordered_set>

namespace game {

LocationTable::LocationTable(unsigned buckWidth, unsigned buckHeight)
    : buckWidth(buckWidth), buckHeight(buckHeight), numBuckW(view::gridWidth / buckWidth),
      numBuckH(view::gridHeight / buckHeight)
{
    // TODO: make static
    bucketGrid_ = std::vector<std::vector<Entity>>(numBuckW * numBuckH, std::vector<Entity>());
}

void LocationTable::update(const Registry &registry)
{
    for (auto bucket : bucketGrid_) {
        bucket.clear(); // Leaves the capacity() of the vector unchanged
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
        unsigned buckIx = (unsigned)std::max(0.0f, (position.x / view::gridWidth));
        unsigned buckIy = (unsigned)std::max(0.0f, (position.y / view::gridHeight));
        getBucket(buckIx, buckIy).emplace_back(entity);

        for (float y = position.y + buckHeight; y <= (position.y + sprite.height); y += buckHeight) {
            for (float x = position.x; x <= (position.x + sprite.width); x += buckWidth) {
                buckIy = std::min((unsigned)(y / buckHeight), numBuckH - 1);
                buckIx = std::min((unsigned)(x / buckWidth), numBuckW - 1);
                getBucket(buckIx, buckIy).emplace_back(entity);
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
std::unordered_set<Entity> LocationTable::getEntitiesNear(const float x, const float y, float radius)
{
    // min/max(): better save than sorry - making no assumtions of logic positioning entities/sprites
    std::unordered_set<Entity> inRange;
    unsigned buckIx = (unsigned)(std::max(0.0f, x - radius) / (float)buckWidth);
    unsigned buckIy = (unsigned)(std::max(0.0f, y - radius) / (float)buckHeight);
    for (; (float)buckIy <= (y + radius); buckIy += view::gridHeight) {
        for (; (float)buckIx <= (x + radius); buckIx += view::gridWidth) {
            // unordered_set removes duplicate entities introduced by update
            inRange.insert_range(cgetBucket(std::min(buckIx, numBuckW - 1), std::min(buckIy, numBuckH - 1)));
        }
    }
    return inRange;
}

} // namespace game