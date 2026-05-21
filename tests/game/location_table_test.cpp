#define TESTING
#include "controller/debug/debug_context.hpp"
#include "controller/input/input_state.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sprite.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/game.hpp"
#include "shared/test_fixture.hpp"
#include "shared/util.hpp"
#include <SFML/Graphics/Rect.hpp>

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <iostream>

void printLTtable(game::LocationTableTester &ltSpy)
{
    for (size_t l = 0; l < ltSpy.getNumBuckH(); l++) {
        for (size_t i = 0; i < ltSpy.getNumBuckW(); i++) {
            std::cout << "(" << l << ", " << i << "): ";
            for (auto e : *ltSpy.getBucket(i, l)) {
                std::cout << e << " ";
            }
            std::cout << std::endl;
        }
    }
    fflush(stdout);
}

// Use tag to only test this TEST_CASE: ./roguelike_test_runner "[location-table]"
TEST_CASE("test location table", "[location-table]")
{
    using namespace game;
    // Tests assume grid dims are integers and dividable by 2
    REQUIRE(std::fmod(view::gridWidth, 2) == 0.0f);
    REQUIRE(std::fmod(view::gridHeight, 2) == 0.0f);

    const unsigned numCellsX = 4;
    const unsigned numCellsY = 4;
    assert(numCellsX % 2 == 0); // assumed by sections
    assert(numCellsY % 2 == 0); // assumed by sections

    LocationTable locTab(view::gridWidth / numCellsX, view::gridHeight / numCellsY);
    LocationTableTester ltSpy(locTab);
    REQUIRE(ltSpy.getNumBuckH() == numCellsY);
    REQUIRE(ltSpy.getNumBuckW() == numCellsX);

    // Sprite dimension such that they can fit in only one bucket
    const float sprBuckPadd = 0.1;
    const float sprtWidth = (float)ltSpy.getBuckWidth() * (1 - sprBuckPadd * 2);
    const float sprtHeight = (float)ltSpy.getBuckHeight() * (1 - sprBuckPadd * 2);
    assert(sprtWidth < ltSpy.getBuckWidth());   // assumed by sections
    assert(sprtHeight < ltSpy.getBuckHeight()); // assumed by sections
    const float sprtXOffset = (float)ltSpy.getBuckWidth() * sprBuckPadd;
    const float sprtYOffset = (float)ltSpy.getBuckHeight() * sprBuckPadd;
    // std::cout << "sprite width: " << sprtWidth << std::endl;
    // std::cout << "sprite height: " << sprtHeight << std::endl;
    Entity eLookup[numCellsX][numCellsY];
    Registry registry;
    for (unsigned x = 0; x < numCellsX; x++) {
        for (unsigned y = 0; y < numCellsY; y++) {
            Entity e = registry.createEntity();
            eLookup[x][y] = e;
            float eX = x * ltSpy.getBuckWidth();
            float eY = y * ltSpy.getBuckHeight();
            registry.addComponent<EnemyTag>(e, {});
            registry.addComponent<Position>(e, {eX + sprtXOffset, eY + sprtYOffset});
            // auto p =
            // std::cout << "xXY: " << eX << "-" << eY << std::endl;
            // std::cout << "pos: " << p.x << "-" << p.y << std::endl;
            registry.addComponent<Velocity>(e, {0, 0});
            registry.addComponent<Sprite>(e, game::Sprite{.width = sprtWidth, .height = sprtHeight});
        }
    }
    locTab.update(registry);

    SECTION("one entity per cell")
    {
        for (unsigned x = 0; x < numCellsX; x++) {
            for (unsigned y = 0; y < numCellsY; y++) {
                auto entities = locTab.getEntitiesNear(x * ltSpy.getBuckWidth(), y * ltSpy.getBuckHeight(), 0);
                REQUIRE(entities.size() == 1);
            }
        }
    }

    SECTION("update without entity position changes results in same internal state")
    {
        std::vector<Entity> before;
        for (auto &row : ltSpy.getGrid()) {
            before.append_range(*row.get());
        }
        locTab.update(registry);
        std::vector<Entity> after;
        for (auto &row : ltSpy.getGrid()) {
            after.append_range(*row.get());
        }
        REQUIRE(before == after);
    }

    SECTION("Getting position outside of grid (x,y 'overflow')")
    {
        auto entities = locTab.getEntitiesNear(view::gridWidth * 2, view::gridHeight * 2, 0);
        REQUIRE(entities.size() == 1);
        REQUIRE(entities.contains(eLookup[numCellsX - 1][numCellsY - 1]));
    }

    SECTION("Getting position outside of grid (x,y 'underflow')")
    {
        auto entities = locTab.getEntitiesNear(-view::gridWidth, -view::gridHeight, 0);
        REQUIRE(entities.size() == 1);
        REQUIRE(entities.contains(eLookup[0][0]));
    }

    SECTION("Sprite (its size smaller than one bucket) overlaps with four buckets")
    {
        // Add entity on grid centered four-bucket-intersection-point
        Entity e = registry.createEntity();
        float eX = (numCellsX / 2) * ltSpy.getBuckWidth() - ltSpy.getBuckWidth() / 2;
        float eY = (numCellsY / 2) * ltSpy.getBuckHeight() - ltSpy.getBuckHeight() / 2;
        registry.addComponent<EnemyTag>(e, {});
        registry.addComponent<Position>(e, {eX + sprtXOffset, eY + sprtYOffset});
        registry.addComponent<Velocity>(e, {0, 0});
        registry.addComponent<Sprite>(e, game::Sprite{.width = sprtWidth, .height = sprtHeight});
        locTab.update(registry);

        // Check internal state
        for (unsigned x = 0; x < numCellsX; x++) {
            for (unsigned y = 0; y < numCellsY; y++) {
                auto b = ltSpy.getBucket(x, y);
                auto numB = std::count(b->cbegin(), b->cend(), e);
                if (x >= 1 && x <= 2 && y >= 1 && y <= 2) {
                    REQUIRE(numB == 1);
                } else {
                    REQUIRE(numB == 0);
                }
            }
        }
        // Check interface output
        for (unsigned x = 0; x < numCellsX; x++) {
            for (unsigned y = 0; y < numCellsY; y++) {
                auto ents = locTab.getEntitiesNear(ltSpy.getBuckWidth() * x, ltSpy.getBuckHeight() * y, 0);
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
        Entity e = registry.createEntity();
        float eX = 0;
        float eY = 0;
        registry.addComponent<EnemyTag>(e, {});
        registry.addComponent<Position>(e, {eX + sprtXOffset, eY + sprtYOffset});
        registry.addComponent<Velocity>(e, {0, 0});
        registry.addComponent<Sprite>(e, game::Sprite{.width = (float)ltSpy.getBuckWidth() * 2, .height = sprtHeight});
        locTab.update(registry);

        // Check internal state
        for (unsigned x = 0; x < numCellsX; x++) {
            for (unsigned y = 0; y < numCellsY; y++) {
                auto b = ltSpy.getBucket(x, y);
                auto numB = std::count(b->cbegin(), b->cend(), e);
                if (x <= 2 && y == 0) {
                    REQUIRE(numB == 1);
                } else {
                    REQUIRE(numB == 0);
                }
            }
        }
        // Check interface output
        for (unsigned x = 0; x < numCellsX; x++) {
            for (unsigned y = 0; y < numCellsY; y++) {
                auto ents = locTab.getEntitiesNear(ltSpy.getBuckWidth() * x, ltSpy.getBuckHeight() * y, 0);
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
        Entity e = registry.createEntity();
        float eX = 0;
        float eY = 0;
        registry.addComponent<EnemyTag>(e, {});
        registry.addComponent<Position>(e, {eX + sprtXOffset, eY + sprtYOffset});
        registry.addComponent<Velocity>(e, {0, 0});
        registry.addComponent<Sprite>(e, game::Sprite{.width = sprtWidth, .height = (float)ltSpy.getBuckHeight() * 2});
        locTab.update(registry);

        // Check internal state
        for (unsigned x = 0; x < numCellsX; x++) {
            for (unsigned y = 0; y < numCellsY; y++) {
                auto b = ltSpy.getBucket(x, y);
                auto numB = std::count(b->cbegin(), b->cend(), e);
                if (x == 0 && y <= 2) {
                    REQUIRE(numB == 1);
                } else {
                    REQUIRE(numB == 0);
                }
            }
        }
        // Check interface output
        for (unsigned x = 0; x < numCellsX; x++) {
            for (unsigned y = 0; y < numCellsY; y++) {
                auto ents = locTab.getEntitiesNear(ltSpy.getBuckWidth() * x, ltSpy.getBuckHeight() * y, 0);
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