#include "controller/debug/debug_context.hpp"
#include "controller/input/input_state.hpp"
#include "controller/persistence/persistence_manager.hpp"
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

TEST_CASE_METHOD(TestFixture, "Game update returns false while player is alive")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;

    // ACT
    bool isGameOver = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE_FALSE(isGameOver);
}

TEST_CASE_METHOD(TestFixture, "Game update returns true when no player exists anymore")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext &debug = controller::DebugContext::get();
    debug.active = true;

    game::GameDebugSession &session = game.getDebugSession();
    session.isPlayerDestructionRequested = true;

    // ACT
    bool isGameOver = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(isGameOver);
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
    bool isGameOver = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE_FALSE(isGameOver);
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
    bool isGameOver = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE_FALSE(isGameOver);
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
    bool isGameOver = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(isGameOver);
    REQUIRE_FALSE(session.isPlayerDestructionRequested);
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
    bool isGameOver = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE_FALSE(isGameOver);
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

    REQUIRE_FALSE(controller::PersistenceManager::hasSavedGame());

    // ACT
    bool isGameOver = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE_FALSE(isGameOver);
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

    REQUIRE_FALSE(controller::PersistenceManager::hasSavedGame());

    // ACT
    bool isGameOver = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE_FALSE(isGameOver);
    REQUIRE(session.isSaveGameRequested);
    REQUIRE_FALSE(controller::PersistenceManager::hasSavedGame());
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
    bool isGameOver = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE_FALSE(isGameOver);

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
    bool isGameOver = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE_FALSE(isGameOver);

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
    REQUIRE(!view.overlayItems.empty());
    REQUIRE(!view.worldItems.empty());
    REQUIRE(view.cameraX == 0.0f);
    REQUIRE(view.cameraY == 0.0f);
}

TEST_CASE_METHOD(TestFixture, "Game loadFromPersistedGame applies persisted values")
{
    game::Game game;

    controller::PersistedGame persistedGame;
    persistedGame.stage = 12;
    persistedGame.wave = 5;
    persistedGame.currency = 1234;
    persistedGame.playerStats.speed = 333.0f;

    game.loadFromPersistedGame(persistedGame);

    const controller::PersistedGame snapshot = game.getPersistedGame();
    REQUIRE(snapshot.stage == 12);
    REQUIRE(snapshot.wave == 5);
    REQUIRE(snapshot.currency == 1234);
    REQUIRE(snapshot.playerStats.speed == 333.0f);
}