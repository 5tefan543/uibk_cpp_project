#include "game/location_table.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sprite.hpp"
#include "game/ecs/entity.hpp"
#include <vector>

namespace game {

void LocationTable::update(const Registry &registry)
{
    // bool cleanup = false;
    auto entities = registry.view<Sprite, Position>();
    for (auto entity : entities) {
        const Position &position = registry.getComponent<Position>(entity);
        const Sprite &sprite = registry.getComponent<Sprite>(entity);

        const unsigned buckIx = (unsigned)((position.y + (sprite.width / 2)) / (float)buckGrdHeight);
        const unsigned buckIy = (unsigned)((position.x + (sprite.height / 2)) / (float)buckGrdWidth);
        bucketGrid_[buckIx][buckIy]->emplace_back(entity);

        // ### Not-thought-through idea how to figure out if we want to free up memory.
        // ### For one wave worst case with no cleanup and N entities max:
        // ### numBuckets := (gridWidth / buckGrdWidth) * (gridHeight / buckGrdHeight)
        // ### -> numBuckets * N * sizeof(Entity) Bytes
        // ### -> numBuckets * N * 4 Bytes
        // ### -> (1920 / 30) * (1080 / 30) * 4 Bytes * 10 000 = 92.16MB
        // ### ...which seems to be ok?
        // ### Another aspect: the overall amount of entities per waves/stages will never decrease with new
        // ### levels/stages so in the long run there will always be a possible situation for any bucket where the
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

std::vector<Entity> LocationTable::getEntitiesNear(const float x, const float y)
{
    // Entities (smaller than bucket size) could overlap with 4 buckets at the same time -> return 3x3 buckets around
    // bucket for x,y
    // TODO: implement solution for entities with sprites larger than bucket size
    std::vector<Entity> inRange;
    const long buckIxCenter = (unsigned)(x / (float)buckGrdHeight);
    const long buckIyCenter = (unsigned)(y / (float)buckGrdHeight);
    for (long xoff = -1; xoff <= 1; xoff++) {
        for (long yoff = -1; yoff <= 1; yoff++) {
            const long buckIx = buckIxCenter + xoff;
            const long buckIy = buckIyCenter + yoff;
            if (buckIx >= 0 && buckIy >= 0) {
                inRange.append_range(*bucketGrid_[buckIx][buckIy].get());
            }
        }
    }
    return inRange;
}

} // namespace game