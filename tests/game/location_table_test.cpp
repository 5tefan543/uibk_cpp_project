#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/location_table.hpp"
#include "view/grid.hpp"

#include "view/sprite.hpp"
#include <SFML/Graphics/Rect.hpp>

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

// #include <iostream>
// void printLTtable(game::LocationTableTester &ltSpy)
// {
//     for (size_t l = 0; l < ltSpy.getNumBuckH(); l++) {
//         for (size_t i = 0; i < ltSpy.getNumBuckW(); i++) {
//             std::cout << "(" << l << ", " << i << "): ";
//             for (auto e : *ltSpy.getBucket(i, l)) {
//                 std::cout << e << " ";
//             }
//             std::cout << std::endl;
//         }
//     }
//     fflush(stdout);
// }

// Use tags to only test this TEST_CASE: ./roguelike_test_runner "[location-table]"
TEST_CASE("test location table", "[location-table]")
{
    using namespace game;
    using Sprite = view::Sprite;
    // Tests assume grid dims are integers and dividable by 2
    REQUIRE(std::fmod(view::gridWidth, 2) == 0.0f);
    REQUIRE(std::fmod(view::gridHeight, 2) == 0.0f);

    constexpr const Vec2<unsigned> numCells = {4, 4};
    assert(numCells.x % 2 == 0); // assumed by sections
    assert(numCells.y % 2 == 0); // assumed by sections

    LocationTable lt(numCells);
    REQUIRE(lt.numBuckets == numCells);

    // Sprite dimension such that they can fit in only one bucket
    const float sprBuckPadd = 0.1;
    const Vec2<float> spriteSize = {lt.bucketSize * (1 - sprBuckPadd * 2)};
    assert(spriteSize.x < lt.bucketSize.x); // assumed by sections
    assert(spriteSize.y < lt.bucketSize.y); // assumed by sections
    const Vec2<float> sprtOffset = lt.bucketSize * sprBuckPadd;
    Entity eLookup[numCells.x][numCells.y];
    Registry registry;

    auto addEntity = [&](const Vec2<float> rawPosWithoutOffset,
                         std::optional<Vec2<float>> sprtSize = std::nullopt) mutable -> Entity {
        Entity e = registry.createEntity();
        const Vec2<float> p = rawPosWithoutOffset + sprtOffset;
        registry.addComponent<EnemyTag>(e, {});
        registry.addComponent<Position>(e, {p.x, p.y});
        registry.addComponent<Velocity>(e, {0, 0});
        if (sprtSize.has_value()) {
            registry.addComponent<Sprite>(e, Sprite{.width = sprtSize.value().x, .height = sprtSize.value().y});
        } else {
            registry.addComponent<Sprite>(e, Sprite{.width = spriteSize.x, .height = spriteSize.y});
        }
        lt.update(registry);
        return e;
    };

    // Grid base state for all SECTIONs
    for (unsigned x = 0; x < numCells.x; x++) {
        for (unsigned y = 0; y < numCells.y; y++) {
            eLookup[x][y] = addEntity(lt.bucketSize * Vec2{x, y}.into<float>());
        }
    }

    SECTION("one entity per cell")
    {
        for (unsigned x = 0; x < numCells.x; x++) {
            for (unsigned y = 0; y < numCells.y; y++) {
                auto entities = lt.getEntitiesNear((lt.numBuckets * Vec2{x, y}).into<float>(), 0);
                REQUIRE(entities.size() == 1);
            }
        }
    }

    SECTION("update without entity position changes results in same internal state")
    {
        std::vector<Entity> before;
        for (auto &row : lt.cgetGrid()) {
            before.append_range(*row.get());
        }
        lt.update(registry);
        std::vector<Entity> after;
        for (auto &row : lt.cgetGrid()) {
            after.append_range(*row.get());
        }
        REQUIRE(before == after);
    }

    SECTION("Getting position outside of grid (x,y 'overflow')")
    {
        auto entities = lt.getEntitiesNear(Vec2{view::gridWidth, view::gridHeight} * 2, 0);
        REQUIRE(entities.size() == 1);
        REQUIRE(entities.contains(eLookup[numCells.x - 1][numCells.y - 1]));
    }

    SECTION("Getting position outside of grid (x,y 'underflow')")
    {
        auto entities = lt.getEntitiesNear(Vec2{-view::gridWidth, -view::gridHeight}, 0);
        REQUIRE(entities.size() == 1);
        REQUIRE(entities.contains(eLookup[0][0]));
    }

    SECTION("Setting position outside of grid (x,y 'underflow')")
    {
        Entity eNew = addEntity(Vec2{0.0f, 0.0f} - (lt.bucketSize * 2));

        for (unsigned x = 0; x < numCells.x; x++) {
            for (unsigned y = 0; y < numCells.y; y++) {
                auto ents = lt.getEntitiesNear(Vec2{lt.bucketSize.x, lt.bucketSize.y} * Vec2{x, y}.into<float>(), 0);
                if (x == 0 && y == 0) {
                    REQUIRE(ents.size() == 2);
                    REQUIRE(ents.contains(eLookup[x][y]));
                    REQUIRE(ents.contains(eNew));
                } else {
                    REQUIRE(ents.size() == 1);
                    REQUIRE(ents.contains(eLookup[x][y]));
                }
            }
        }
    }

    SECTION("Setting position outside of grid (x,y 'overflow')")
    {
        Entity eNew = addEntity((lt.numBuckets.into<float>() * lt.bucketSize) + (lt.bucketSize * 2));

        for (unsigned x = 0; x < numCells.x; x++) {
            for (unsigned y = 0; y < numCells.y; y++) {
                auto ents = lt.getEntitiesNear(Vec2{lt.bucketSize.x, lt.bucketSize.y} * Vec2{x, y}.into<float>(), 0);
                if (x == numCells.x - 1 && y == numCells.y - 1) {
                    REQUIRE(ents.size() == 2);
                    REQUIRE(ents.contains(eLookup[x][y]));
                    REQUIRE(ents.contains(eNew));
                } else {
                    REQUIRE(ents.size() == 1);
                    REQUIRE(ents.contains(eLookup[x][y]));
                }
            }
        }
    }

    SECTION("Sprite (its size smaller than one bucket) overlaps with four buckets")
    {
        // Add entity on grid centered four-bucket-intersection-point
        Entity e = addEntity((numCells.into<float>() / 2) * lt.bucketSize - (lt.bucketSize / 2));

        // Check internal state
        for (unsigned x = 0; x < numCells.x; x++) {
            for (unsigned y = 0; y < numCells.y; y++) {
                auto b = lt.cgetBucket(x, y);
                auto numB = std::count(b->cbegin(), b->cend(), e);
                if (x >= 1 && x <= 2 && y >= 1 && y <= 2) {
                    REQUIRE(numB == 1);
                } else {
                    REQUIRE(numB == 0);
                }
            }
        }
        // Check interface output
        for (unsigned x = 0; x < numCells.x; x++) {
            for (unsigned y = 0; y < numCells.y; y++) {
                auto ents = lt.getEntitiesNear(Vec2{lt.bucketSize.x, lt.bucketSize.y} * Vec2{x, y}.into<float>(), 0);
                if (x >= 1 && x <= 2 && y >= 1 && y <= 2) {
                    REQUIRE(ents.size() == 2);
                    REQUIRE(ents.contains(e));
                } else {
                    REQUIRE(ents.size() == 1);
                    REQUIRE(!ents.contains(e));
                }
            }
        }
    }

    SECTION("Sprite (its width wider than one bucket) overlaps with three buckets")
    {
        Entity e = addEntity({0, 0}, std::optional(Vec2<float>{lt.bucketSize.x * 2, spriteSize.y}));

        // Check internal state
        for (unsigned x = 0; x < numCells.x; x++) {
            for (unsigned y = 0; y < numCells.y; y++) {
                auto b = lt.cgetBucket(x, y);
                auto numB = std::count(b->cbegin(), b->cend(), e);
                if (x <= 2 && y == 0) {
                    REQUIRE(numB == 1);
                } else {
                    REQUIRE(numB == 0);
                }
            }
        }
        // Check interface output
        for (unsigned x = 0; x < numCells.x; x++) {
            for (unsigned y = 0; y < numCells.y; y++) {
                auto ents = lt.getEntitiesNear(Vec2{lt.bucketSize.x, lt.bucketSize.y} * Vec2{x, y}.into<float>(), 0);
                if (x <= 2 && y == 0) {
                    REQUIRE(ents.size() == 2);
                    REQUIRE(ents.contains(e));
                } else {
                    REQUIRE(ents.size() == 1);
                    REQUIRE(!ents.contains(e));
                }
            }
        }
    }

    SECTION("Sprite (its height higher than one bucket) overlaps with three buckets")
    {
        Entity e = addEntity({0, 0}, std::optional(Vec2<float>{spriteSize.x, lt.bucketSize.y * 2}));

        // Check internal state
        for (unsigned x = 0; x < numCells.x; x++) {
            for (unsigned y = 0; y < numCells.y; y++) {
                auto b = lt.cgetBucket(x, y);
                auto numB = std::count(b->cbegin(), b->cend(), e);
                if (x == 0 && y <= 2) {
                    REQUIRE(numB == 1);
                } else {
                    REQUIRE(numB == 0);
                }
            }
        }
        // Check interface output
        for (unsigned x = 0; x < numCells.x; x++) {
            for (unsigned y = 0; y < numCells.y; y++) {
                auto ents = lt.getEntitiesNear(Vec2{lt.bucketSize.x, lt.bucketSize.y} * Vec2{x, y}.into<float>(), 0);
                if (x == 0 && y <= 2) {
                    REQUIRE(ents.size() == 2);
                    REQUIRE(ents.contains(e));
                } else {
                    REQUIRE(ents.size() == 1);
                    REQUIRE(!ents.contains(e));
                }
            }
        }
    }
}