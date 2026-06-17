#include "game/ecs/components/sound.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/sound_system.hpp"
#include "shared/audio_test_util.hpp"
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

TEST_CASE_METHOD(TestFixture, "SoundSystem update removes SoundComponent after successful playback")
{
    test::writeMinimalWav("sound.wav");

    game::Registry registry;
    game::SoundSystem system;

    game::Entity entity = registry.createEntity();
    registry.addComponent<game::SoundComponent>(entity, {.name = "sound.wav"});

    REQUIRE_NOTHROW(system.update(registry));
    REQUIRE_FALSE(registry.hasComponent<game::SoundComponent>(entity));
}

TEST_CASE_METHOD(TestFixture, "SoundSystem update processes all entities with SoundComponent")
{
    test::writeMinimalWav("sound.wav");

    game::Registry registry;
    game::SoundSystem system;

    game::Entity e1 = registry.createEntity();
    game::Entity e2 = registry.createEntity();
    registry.addComponent<game::SoundComponent>(e1, {.name = "sound.wav"});
    registry.addComponent<game::SoundComponent>(e2, {.name = "sound.wav"});

    REQUIRE_NOTHROW(system.update(registry));
    REQUIRE_FALSE(registry.hasComponent<game::SoundComponent>(e1));
    REQUIRE_FALSE(registry.hasComponent<game::SoundComponent>(e2));
}

TEST_CASE_METHOD(TestFixture, "SoundSystem update leaves entities without SoundComponent unchanged")
{
    game::Registry registry;
    game::SoundSystem system;

    game::Entity entity = registry.createEntity(); // no SoundComponent

    REQUIRE_NOTHROW(system.update(registry));
    REQUIRE(registry.isEntityAlive(entity));
}
