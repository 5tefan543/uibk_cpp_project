#include "controller/debug/debug_context.hpp"
#include "controller/input/input_state.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/game.hpp"
#include "shared/test_fixture.hpp"
#include "shared/util.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(TestFixture, "Game can be constructed")
{
    REQUIRE_NOTHROW(game::Game{});
}

TEST_CASE_METHOD(TestFixture, "Game update returns correct StateTransitionAction while player is alive")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;

    // ACT
    auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::None);
}

TEST_CASE_METHOD(TestFixture, "Game update returns ReplaceCurrentWithGameOverMenu when no player exists anymore")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext &debug = controller::DebugContext::get();
    debug.active = true;

    game::GameDebugSession &session = game.getDebugSession();
    session.isPlayerDestructionRequested = true;

    // ACT
    auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::ReplaceCurrentWithGameOverMenu);
}

TEST_CASE_METHOD(TestFixture, "Game update resets stage/wave reload request when debug is active")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext &debug = controller::DebugContext::get();
    debug.active = true;

    game::GameDebugSession &session = game.getDebugSession();
    session.isStageWaveReloadRequested = true;

    // ACT
    auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::None);
    REQUIRE_FALSE(session.isStageWaveReloadRequested);
}

TEST_CASE_METHOD(TestFixture, "Game update keeps stage/wave reload request unchanged when debug is inactive")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext &debug = controller::DebugContext::get();
    debug.active = false;

    game::GameDebugSession &session = game.getDebugSession();
    session.isStageWaveReloadRequested = true;

    // ACT
    auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::None);
    REQUIRE(session.isStageWaveReloadRequested);
}

TEST_CASE_METHOD(TestFixture, "Game update resets player destruction request when debug is active")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext &debug = controller::DebugContext::get();
    debug.active = true;

    game::GameDebugSession &session = game.getDebugSession();
    session.isPlayerDestructionRequested = true;

    // ACT
    auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::ReplaceCurrentWithGameOverMenu);
    REQUIRE_FALSE(session.isPlayerDestructionRequested);
}

/*
==========================================================================
==========================================================================
=============================HERE!!!!!!!!!!!!=============================
==========================================================================
==========================================================================
*/
TEST_CASE_METHOD(TestFixture, "Game update returns PushProgressionStore when a stage boundary wave is defeated")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext &debug = controller::DebugContext::get();
    debug.active = true;
    int wavesPerStage = controller::PersistenceManager::getConfig().wavesPerStage;
    game::GameDebugSession &session = game.getDebugSession();
    session.stage = 1;
    session.wave = wavesPerStage;
    session.isStageWaveReloadRequested = true;

    // Apply stage/wave reload before defeating the enemies of that wave.
    game.update(input, 0.0f);

    // Defeat the current wave by removing all enemies.
    for (game::Entity enemy : session.registry.view<game::EnemyTag>()) {
        session.registry.destroyEntity(enemy);
    }

    // ACT
    const auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::PushProgressionStore);
}

/*
==========================================================================
==========================================================================
=============================HERE!!!!!!!!!!!!=============================
==========================================================================
==========================================================================
*/
TEST_CASE_METHOD(TestFixture,
                 "Game update advances to next wave and returns None when defeated wave is not stage boundary")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext &debug = controller::DebugContext::get();
    debug.active = true;
    int wavesPerStage = controller::PersistenceManager::getConfig().wavesPerStage;
    REQUIRE(wavesPerStage > 1);

    game::GameDebugSession &session = game.getDebugSession();
    session.stage = 1;
    session.wave = wavesPerStage - 1;
    session.isStageWaveReloadRequested = true;

    // Apply stage/wave reload before defeating the enemies of that wave.
    game.update(input, 0.0f);

    for (game::Entity enemy : session.registry.view<game::EnemyTag>()) {
        session.registry.destroyEntity(enemy);
    }

    // ACT
    const auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::None);

    const controller::PersistedGame persisted = game.getPersistedGame();
    const int expectedWave = wavesPerStage;
    const int expectedStage = ((expectedWave - 1) / wavesPerStage) + 1;
    REQUIRE(persisted.wave == expectedWave);
    REQUIRE(game.getDebugSession().stage == expectedStage);
}

TEST_CASE_METHOD(TestFixture, "Game update keeps player destruction request unchanged when debug is inactive")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext &debug = controller::DebugContext::get();
    debug.active = false;

    game::GameDebugSession &session = game.getDebugSession();
    session.isPlayerDestructionRequested = true;

    // ACT
    auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::None);
    REQUIRE(session.isPlayerDestructionRequested);
}

TEST_CASE_METHOD(TestFixture, "Game update saves game on save request and resets request when debug is active")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext &debug = controller::DebugContext::get();
    debug.active = true;

    game::GameDebugSession &session = game.getDebugSession();
    session.isSaveGameRequested = true;

    // ACT
    auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::None);
    REQUIRE_FALSE(session.isSaveGameRequested);
    REQUIRE(controller::PersistenceManager::hasSavedGame());
}

TEST_CASE_METHOD(TestFixture, "Game update keeps save game request unchanged when debug is inactive")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext &debug = controller::DebugContext::get();
    debug.active = false;

    game::GameDebugSession &session = game.getDebugSession();
    session.isSaveGameRequested = true;

    // ACT
    auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::None);
    REQUIRE(session.isSaveGameRequested);
}

TEST_CASE_METHOD(TestFixture, "Game update skips system updates when debug is active and system updates are disabled")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext &debug = controller::DebugContext::get();
    debug.active = true;

    game::GameDebugSession &session = game.getDebugSession();
    session.isSystemUpdateActive = false;

    auto players = session.registry.view<game::PlayerTag>();
    REQUIRE_FALSE(players.empty());

    game::Entity player = players.front();
    REQUIRE(session.registry.hasComponent<game::Position>(player));

    game::Position &positionBefore = session.registry.getComponent<game::Position>(player);
    float positionBeforeX = positionBefore.x;
    float positionBeforeY = positionBefore.y;

    input.rightHeld = true;

    // ACT
    auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::None);

    game::Position &positionAfter = session.registry.getComponent<game::Position>(player);
    REQUIRE(positionAfter.x == positionBeforeX);
    REQUIRE(positionAfter.y == positionBeforeY);
}

TEST_CASE_METHOD(TestFixture,
                 "Game update still runs system updates when debug is inactive even if system updates are disabled")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext &debug = controller::DebugContext::get();
    debug.active = false;

    game::GameDebugSession &session = game.getDebugSession();
    session.isSystemUpdateActive = false;

    auto players = session.registry.view<game::PlayerTag>();
    REQUIRE_FALSE(players.empty());

    game::Entity player = players.front();
    REQUIRE(session.registry.hasComponent<game::Position>(player));

    game::Position &positionBefore = session.registry.getComponent<game::Position>(player);
    float positionBeforeX = positionBefore.x;
    float positionBeforeY = positionBefore.y;

    input.rightHeld = true;

    // ACT
    auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::None);

    game::Position &positionAfter = session.registry.getComponent<game::Position>(player);
    REQUIRE(positionAfter.x != positionBeforeX);
    REQUIRE(positionAfter.y == positionBeforeY);
}

TEST_CASE_METHOD(TestFixture, "Game getView returns correct view")
{
    // ARRANGE
    game::Game game;
    view::View view;

    // ACT
    game.updateView(view);

    // ASSERT
    REQUIRE(!view.nodes.empty());
    REQUIRE(view.cameraX == 0.0f);
    REQUIRE(view.cameraY == 0.0f);
}

/*
==========================================================================
==========================================================================
=============================HERE!!!!!!!!!!!!=============================
==========================================================================
==========================================================================
*/
TEST_CASE_METHOD(TestFixture, "Game loadFromPersistedGame applies persisted values")
{
    game::Game game;

    controller::PersistedGame persistedGame;
    persistedGame.wave = 5;
    persistedGame.currency = 1234;
    persistedGame.playerStats.speed = 333.0f;

    game.loadFromPersistedGame(persistedGame);

    const controller::PersistedGame snapshot = game.getPersistedGame();
    REQUIRE(snapshot.wave == 6); // wave gets incremented after loading
    REQUIRE(game.getDebugSession().stage == 3);
    REQUIRE(snapshot.currency == 1234);
    REQUIRE(snapshot.playerStats.speed == 333.0f);
}