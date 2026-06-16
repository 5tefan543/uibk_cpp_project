#include "game/ecs/components/sound.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/sound_system.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(TestFixture, "SoundSystem update is a no-op when there are no sound components")
{
    game::Registry registry;
    game::SoundSystem system;

    game::Entity entity = registry.createEntity();
    (void)entity;

    REQUIRE_NOTHROW(system.update(registry));
}

TEST_CASE_METHOD(TestFixture, "SoundSystem update throws on invalid sound path and keeps component")
{
    game::Registry registry;
    game::SoundSystem system;

    game::Entity entity = registry.createEntity();
    registry.addComponent<game::SoundComponent>(entity, {.name = "this/path/does/not/exist.wav"});

    REQUIRE_THROWS(system.update(registry));
    REQUIRE(registry.hasComponent<game::SoundComponent>(entity));
}
