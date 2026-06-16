// #include "controller/input/input_state.hpp"
// #include "controller/persistence/persistence_manager.hpp"
// #include "game/ecs/components/animation.hpp"
// #include "game/ecs/components/damage.hpp"
// #include "game/ecs/components/hitbox.hpp"
// #include "game/ecs/components/player_tag.hpp"
// #include "game/ecs/components/position.hpp"
// #include "game/ecs/components/stats.hpp"
// #include "game/ecs/components/velocity.hpp"
// #include "game/ecs/registry.hpp"
// #include "game/ecs/systems/input_system.hpp"
// #include "shared/test_fixture.hpp"
// #include "view/sprite.hpp"

// #include <catch2/catch_test_macros.hpp>
// #include <cmath>

// TEST_CASE_METHOD(TestFixture, "InputSystem sets player velocity from input")
// {
//     // ARRANGE
//     game::Registry registry;
//     game::InputSystem system;

//     game::Entity player = registry.createEntity();
//     game::PlayerStats playerStats;
//     playerStats.moveSpeed = 200.0f;
//     registry.addComponent<game::PlayerStats>(player, playerStats);
//     registry.addComponent<game::Velocity>(player, {0.0f, 0.0f});

//     controller::InputState input;
//     input.leftHeld = true;
//     input.upHeld = true;

//     // ACT
//     const config::GameConfig &config = controller::PersistenceManager::getConfig();
//     system.update(registry, config, input, dummyDeltaTime);

//     // ASSERT
//     const auto &velocity = registry.getComponent<game::Velocity>(player);
//     auto speed = std::sqrt(std::pow(velocity.x, 2) + std::pow(velocity.x, 2));
//     REQUIRE(std::abs(speed) - playerStats.moveSpeed < 0.001f);
// }

// TEST_CASE_METHOD(TestFixture, "InputSystem resets old velocity before applying new input")
// {
//     // ARRANGE
//     game::Registry registry;
//     game::InputSystem system;

//     game::Entity player = registry.createEntity();
//     game::PlayerStats playerStats;
//     playerStats.moveSpeed = 200.0f;
//     registry.addComponent<game::PlayerStats>(player, playerStats);
//     registry.addComponent<game::Velocity>(player, {999.0f, 999.0f});

//     controller::InputState input;

//     // ACT
//     const config::GameConfig &config = controller::PersistenceManager::getConfig();
//     system.update(registry, config, input, dummyDeltaTime);

//     // ASSERT
//     const auto &velocity = registry.getComponent<game::Velocity>(player);
//     REQUIRE(velocity.x == 0.0f);
//     REQUIRE(velocity.y == 0.0f);
// }

// TEST_CASE_METHOD(TestFixture, "InputSystem does not update entity without PlayerStats")
// {
//     // ARRANGE
//     game::Registry registry;
//     game::InputSystem system;

//     game::Entity notPlayer = registry.createEntity();
//     registry.addComponent<game::Velocity>(notPlayer, {5.0f, 6.0f});

//     controller::InputState input;
//     input.rightHeld = true;
//     input.downHeld = true;

//     // ACT
//     const config::GameConfig &config = controller::PersistenceManager::getConfig();
//     system.update(registry, config, input, dummyDeltaTime);

//     // ASSERT
//     const auto &velocity = registry.getComponent<game::Velocity>(notPlayer);
//     REQUIRE(velocity.x == 5.0f);
//     REQUIRE(velocity.y == 6.0f);
// }

// TEST_CASE_METHOD(TestFixture, "InputSystem opposing directions cancel each other")
// {
//     // ARRANGE
//     game::Registry registry;
//     game::InputSystem system;

//     game::Entity player = registry.createEntity();
//     game::PlayerStats playerStats;
//     playerStats.moveSpeed = 200.0f;
//     registry.addComponent<game::PlayerStats>(player, playerStats);
//     registry.addComponent<game::Velocity>(player, {0.0f, 0.0f});

//     controller::InputState input;
//     input.leftHeld = true;
//     input.rightHeld = true;
//     input.upHeld = true;
//     input.downHeld = true;

//     // ACT
//     const config::GameConfig &config = controller::PersistenceManager::getConfig();
//     system.update(registry, config, input, dummyDeltaTime);

//     // ASSERT
//     const auto &velocity = registry.getComponent<game::Velocity>(player);
//     REQUIRE(velocity.x == 0.0f);
//     REQUIRE(velocity.y == 0.0f);
// }

// TEST_CASE_METHOD(TestFixture, "InputSystem melee attack activates player attack animation override")
// {
//     game::Registry registry;
//     game::InputSystem system;

//     game::Entity player = registry.createEntity();

//     game::PlayerStats playerStats;
//     playerStats.attackSpeed = 2.0f;
//     playerStats.characterType = game::CharacterType::Melee;
//     registry.addComponent<game::PlayerStats>(player, playerStats);
//     registry.addComponent<game::Position>(player, {100.0f, 100.0f});
//     registry.addComponent<game::Velocity>(player, {0.0f, 0.0f});
//     registry.addComponent<game::Animation>(player, {.attackTexturePath = "assets/characters/melee/atk_",
//                                                     .attackFrameDuration = 0.3f,
//                                                     .attackTotalFrames = 2,
//                                                     .attackMoveSpeedMultiplier = 0.5f});

//     controller::InputState input;
//     input.mouseLeftPressed = true;
//     input.mouseGridX = 130.0f;
//     input.mouseGridY = 100.0f;

//     const config::GameConfig &config = controller::PersistenceManager::getConfig();
//     system.update(registry, config, input, 1.0f);

//     const auto &animation = registry.getComponent<game::Animation>(player);
//     REQUIRE(animation.overrideState == game::AnimationOverrideState::Attack);
//     REQUIRE(animation.overrideDirection == game::AnimationDirection::Right);
//     REQUIRE(animation.overrideTimeRemaining > 0.59f);
//     REQUIRE(animation.overrideTimeRemaining < 0.61f);

//     const auto damageEntities = registry.view<game::Damage, game::HitBox>();
//     REQUIRE(damageEntities.size() == 1);

//     for (game::Entity attack : damageEntities) {
//         REQUIRE_FALSE(registry.hasComponent<view::Sprite>(attack));
//         REQUIRE_FALSE(registry.hasComponent<game::Animation>(attack));
//     }
// }

// TEST_CASE_METHOD(TestFixture, "InputSystem reduces movement speed while attack override is active")
// {
//     game::Registry registry;
//     game::InputSystem system;

//     game::Entity player = registry.createEntity();
//     game::PlayerStats playerStats;
//     playerStats.moveSpeed = 200.0f;
//     registry.addComponent<game::PlayerStats>(player, playerStats);
//     registry.addComponent<game::Velocity>(player, {0.0f, 0.0f});
//     registry.addComponent<game::Animation>(
//         player, {.overrideState = game::AnimationOverrideState::Attack, .attackMoveSpeedMultiplier = 0.5f});

//     controller::InputState input;
//     input.rightHeld = true;

//     const config::GameConfig &config = controller::PersistenceManager::getConfig();
//     system.update(registry, config, input, dummyDeltaTime);

//     const auto &velocity = registry.getComponent<game::Velocity>(player);
//     REQUIRE(velocity.x == 100.0f);
//     REQUIRE(velocity.y == 0.0f);
// }

// TEST_CASE_METHOD(TestFixture, "InputSystem ranged attack spawns projectile with expected components")
// {
//     game::Registry registry;
//     game::InputSystem system;

//     const game::Entity player = registry.createEntity();
//     game::PlayerStats playerStats;
//     playerStats.attackSpeed = 1.0f;
//     playerStats.attackRange = 120.0f;
//     playerStats.speedOfAttack = 5.0f;
//     playerStats.characterType = game::CharacterType::Ranged;
//     registry.addComponent<game::PlayerStats>(player, playerStats);
//     registry.addComponent<game::Position>(player, {0.0f, 0.0f});
//     registry.addComponent<game::Velocity>(player, {0.0f, 0.0f});

//     controller::InputState input;
//     input.mouseLeftPressed = true;
//     input.mouseGridX = 100.0f;
//     input.mouseGridY = 100.0f;

//     const config::GameConfig &config = controller::PersistenceManager::getConfig();
//     system.update(registry, config, input, 1.1f);

//     const auto attackEntities =
//         registry.view<game::Damage, game::Velocity, game::HitBox, game::Position, game::PlayerTag, view::Sprite>();
//     REQUIRE(attackEntities.size() == 1);

//     const game::Entity projectile = attackEntities.front();
//     const auto &damage = registry.getComponent<game::Damage>(projectile);
//     const auto *projectileParams = std::get_if<game::ProjectileDamage>(&damage.params);
//     REQUIRE(projectileParams != nullptr);
//     REQUIRE(projectileParams->maxRange == 120.0f);
//     REQUIRE(projectileParams->maxTargets == 1);

//     const auto &velocity = registry.getComponent<game::Velocity>(projectile);
//     REQUIRE(std::abs(velocity.x + 26.843f) < 0.001f);
//     REQUIRE(std::abs(velocity.y - 42.183f) < 0.001f);
// }

// TEST_CASE_METHOD(TestFixture, "InputSystem attack cooldown is strict on boundary and blocks rapid second attack")
// {
//     game::Registry registry;
//     game::InputSystem system;

//     const game::Entity player = registry.createEntity();
//     game::PlayerStats playerStats;
//     playerStats.attackSpeed = 2.0f;
//     playerStats.attackRange = 80.0f;
//     playerStats.speedOfAttack = 6.0f;
//     playerStats.characterType = game::CharacterType::Ranged;
//     registry.addComponent<game::PlayerStats>(player, playerStats);
//     registry.addComponent<game::Position>(player, {0.0f, 0.0f});
//     registry.addComponent<game::Velocity>(player, {0.0f, 0.0f});

//     controller::InputState input;
//     input.mouseLeftPressed = true;
//     input.mouseGridX = 10.0f;
//     input.mouseGridY = 0.0f;

//     const config::GameConfig &config = controller::PersistenceManager::getConfig();

//     system.update(registry, config, input, 0.5f);
//     REQUIRE(registry.view<game::Damage>().empty());

//     system.update(registry, config, input, 0.01f);
//     REQUIRE(registry.view<game::Damage>().size() == 1);

//     system.update(registry, config, input, 0.2f);
//     REQUIRE(registry.view<game::Damage>().size() == 1);
// }

// TEST_CASE_METHOD(TestFixture, "InputSystem melee attack cooldown blocks boundary and rapid follow up")
// {
//     game::Registry registry;
//     game::InputSystem system;

//     const game::Entity player = registry.createEntity();
//     game::PlayerStats playerStats;
//     playerStats.attackSpeed = 2.0f;
//     playerStats.attackRange = 80.0f;
//     playerStats.characterType = game::CharacterType::Melee;
//     registry.addComponent<game::PlayerStats>(player, playerStats);
//     registry.addComponent<game::Position>(player, {10.0f, 10.0f});
//     registry.addComponent<game::Velocity>(player, {0.0f, 0.0f});

//     controller::InputState input;
//     input.mouseLeftPressed = true;
//     input.mouseGridX = 100.0f;
//     input.mouseGridY = 10.0f;

//     const config::GameConfig &config = controller::PersistenceManager::getConfig();

//     system.update(registry, config, input, 0.5f);
//     REQUIRE(registry.view<game::Damage>().empty());

//     system.update(registry, config, input, 0.01f);
//     REQUIRE(registry.view<game::Damage>().size() == 1);

//     system.update(registry, config, input, 0.1f);
//     REQUIRE(registry.view<game::Damage>().size() == 1);
// }