#include "controller/debug/debug_context.hpp"
#include "controller/input/input_state.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/health_bar_state.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/game.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {

std::vector<view::Rectangle> collectWorldRectangles(const view::View &view)
{
    std::vector<view::Rectangle> rectangles;
    for (const auto &node : view.nodes) {
        if (node.mode != view::ViewMode::FixedToWorld) {
            continue;
        }

        if (const auto *rect = std::get_if<view::Rectangle>(&node.element)) {
            rectangles.push_back(*rect);
        }
    }
    return rectangles;
}

bool isColor(const std::optional<view::Color> &color, std::uint8_t r, std::uint8_t g, std::uint8_t b)
{
    if (!color.has_value()) {
        return false;
    }
    return color->red == r && color->green == g && color->blue == b;
}

} // namespace

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
    game.update(input, std::chrono::milliseconds(0));

    // Defeat the current wave by removing all enemies.
    for (game::Entity enemy : session.registry.view<game::EnemyTag>()) {
        session.registry.destroyEntity(enemy);
    }

    // ACT
    const auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::PushProgressionStore);
}

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
    game.update(input, std::chrono::milliseconds(0));

    for (game::Entity enemy : session.registry.view<game::EnemyTag>()) {
        session.registry.destroyEntity(enemy);
    }

    // ACT
    const auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::None);

    const game::PersistedGame persisted = game.getPersistedGame();
    const int expectedWave = wavesPerStage;
    const int expectedStage = ((expectedWave - 1) / wavesPerStage) + 1;
    REQUIRE(persisted.wave == expectedWave);
    REQUIRE(game.getDebugSession().stage == expectedStage);
}

TEST_CASE_METHOD(TestFixture, "Game update cleanup destroys damage entities when wave finishes")
{
    game::Game game;
    controller::InputState input;

    controller::DebugContext &debug = controller::DebugContext::get();
    debug.active = true;

    game::GameDebugSession &session = game.getDebugSession();
    session.wave = 1;
    session.stage = 1;
    session.isStageWaveReloadRequested = true;

    game.update(input, std::chrono::milliseconds(0));

    for (game::Entity enemy : session.registry.view<game::EnemyTag>()) {
        session.registry.destroyEntity(enemy);
    }

    const game::Entity damage = session.registry.createEntity();
    game::Damage damageComponent{
        .amount = 1.0f,
        .pushBackForce = 0.0f,
        .stunChance = 0.0f,
        .kind = game::DamageKind::Projectile,
        .params = game::ProjectileDamage{.speed = 0.0f, .maxRange = 100.0f, .distanceTraveled = 0.0f, .maxTargets = 1}};
    session.registry.addComponent<game::Damage>(damage, damageComponent);
    session.registry.addComponent<game::DamageTag>(damage, {});

    REQUIRE(session.registry.isEntityAlive(damage));

    game.update(input, dummyDeltaTime);

    REQUIRE_FALSE(session.registry.hasComponent<game::Damage>(damage));
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

    const geometry::Vec2<float> positionBefore = session.registry.getComponent<game::Position>(player).p;

    input.rightHeld = true;

    // ACT
    auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::None);

    const auto &positionAfter = session.registry.getComponent<game::Position>(player).p;
    REQUIRE((positionAfter == positionBefore).all());
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

    const geometry::Vec2<float> positionBefore = session.registry.getComponent<game::Position>(player).p;

    input.rightHeld = true;

    // ACT
    auto currentState = game.update(input, dummyDeltaTime);

    // ASSERT
    REQUIRE(currentState == controller::StateTransitionAction::None);

    const auto &positionAfter = session.registry.getComponent<game::Position>(player).p;
    REQUIRE(positionAfter.x != positionBefore.x);
    REQUIRE(positionAfter.y == positionBefore.y);
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
    REQUIRE((view.cameraPosition == geometry::Vec2{0.0f, 0.0f}).all());
}

TEST_CASE_METHOD(TestFixture, "Game updateView with hitbox debug enabled renders hitbox rectangles")
{
    game::Game game;
    controller::InputState input;

    controller::DebugContext &debug = controller::DebugContext::get();
    debug.active = true;
    debug.gameSettings.showHitboxes = false;

    view::View withoutHitboxes;
    game.updateView(withoutHitboxes);
    const size_t nodesWithoutHitboxes = withoutHitboxes.nodes.size();

    game.update(input, std::chrono::milliseconds(0));

    auto hitboxEntities = game.getDebugSession().registry.view<game::HitBox>();
    REQUIRE_FALSE(hitboxEntities.empty());

    debug.gameSettings.showHitboxes = true;
    view::View withHitboxes;
    game.updateView(withHitboxes);

    REQUIRE(withHitboxes.nodes.size() > nodesWithoutHitboxes);
}

TEST_CASE_METHOD(TestFixture, "Game updateView renders player health bar even at full health")
{
    game::Game game;
    game::GameDebugSession &session = game.getDebugSession();

    const game::Entity player = session.registry.view<game::PlayerTag>().front();
    auto &playerStats = session.registry.getComponent<game::PlayerStats>(player);
    auto &playerBar = session.registry.getComponent<game::HealthBarState>(player);
    const auto &playerPos = session.registry.getComponent<game::Position>(player).p;
    const auto &playerSprite = session.registry.getComponent<view::Sprite>(player);

    playerStats.health = playerStats.maxHealth;
    playerBar.initialRedBarNorm = 0.0f;
    playerBar.redBarTimer = 0.0f;

    view::View view;
    game.updateView(view);

    const auto rectangles = collectWorldRectangles(view);
    const float expectedBarY = playerPos.y - 10.0f - 5.0f;

    bool foundBackground = false;
    bool foundGreen = false;
    bool foundRed = false;

    for (const auto &rect : rectangles) {
        const bool samePosition = rect.rect.position.x == playerPos.x && rect.rect.position.y == expectedBarY;
        if (!samePosition) {
            continue;
        }

        if (isColor(rect.fillColor, 0, 0, 0) && rect.thickness == 2.0f) {
            foundBackground = true;
            REQUIRE(rect.rect.size.x == Catch::Approx(playerSprite.rect.size.x));
            REQUIRE(rect.rect.size.y == Catch::Approx(10.0f));
        }

        if (isColor(rect.fillColor, 0, 200, 0) && rect.thickness == 0.0f) {
            foundGreen = true;
            REQUIRE(rect.rect.size.x == Catch::Approx(playerSprite.rect.size.x));
            REQUIRE(rect.rect.size.y == Catch::Approx(10.0f));
        }

        if (isColor(rect.fillColor, 220, 0, 0)) {
            foundRed = true;
        }
    }

    REQUIRE(foundBackground);
    REQUIRE(foundGreen);
    REQUIRE_FALSE(foundRed);
}

TEST_CASE_METHOD(TestFixture, "Game updateView does not render enemy health bar at full health")
{
    game::Game game;
    game::GameDebugSession &session = game.getDebugSession();

    const game::Entity enemy = session.registry.createEntity();
    session.registry.addComponent<game::EnemyTag>(enemy, {});
    session.registry.addComponent<game::Position>(enemy, {{700.0f, 400.0f}});
    session.registry.addComponent<view::Sprite>(enemy, {.rect = {{0.0f, 0.0f}, {80.0f, 40.0f}}});

    game::EnemyStats stats;
    stats.maxHealth = 100.0f;
    stats.health = 100.0f;
    session.registry.addComponent<game::EnemyStats>(enemy, stats);
    session.registry.addComponent<game::HealthBarState>(enemy, {});

    view::View view;
    game.updateView(view);

    const auto rectangles = collectWorldRectangles(view);
    const float enemyBarY = 400.0f - 10.0f - 5.0f;

    bool enemyBarFound = false;
    for (const auto &rect : rectangles) {
        if (rect.rect.position.x == 700.0f && rect.rect.position.y == enemyBarY) {
            enemyBarFound = true;
            break;
        }
    }

    REQUIRE_FALSE(enemyBarFound);
}

TEST_CASE_METHOD(TestFixture, "Game updateView correctly overlaps sprites")
{
    game::Game game;
    game::GameDebugSession &session = game.getDebugSession();

    const game::Entity enemy = session.registry.createEntity();
    session.registry.addComponent<game::EnemyTag>(enemy, {});
    session.registry.addComponent<game::Position>(enemy, {{700.0f, 400.0f}});
    session.registry.addComponent<view::Sprite>(enemy, {.rect = {{0.0f, 0.0f}, {80.0f, 40.0f}}});

    const game::Entity boss = session.registry.createEntity();
    session.registry.addComponent<game::EnemyTag>(boss, {});
    session.registry.addComponent<game::Position>(boss, {{700.0f, 400.0f}});
    session.registry.addComponent<view::Sprite>(boss, {.rect = {{0.0f, 0.0f}, {81.0f, 40.0f}}});

    const game::Entity player = session.registry.createEntity();
    session.registry.addComponent<game::PlayerTag>(player, {});
    session.registry.addComponent<game::Position>(player, {{700.0f, 400.0f}});
    session.registry.addComponent<view::Sprite>(player, {.rect = {{0.0f, 0.0f}, {82.0f, 40.0f}}});

    const game::Entity projectile = session.registry.createEntity();
    session.registry.addComponent<game::PlayerTag>(projectile, {});
    session.registry.addComponent<game::Position>(projectile, {{700.0f, 400.0f}});
    session.registry.addComponent<view::Sprite>(projectile, {.rect = {{0.0f, 0.0f}, {83.0f, 40.0f}}});
    game::Damage damage = {
        .amount = 42,
        .pushBackForce = 0.0f,
        .stunChance = 0.0f,
        .kind = game::DamageKind::Projectile,
        .params =
            game::ProjectileDamage{
                .speed = 0.0f,
                .maxRange = 0.0f,
                .distanceTraveled = 0.0f,
                .maxTargets = 1,
            },
    };
    session.registry.addComponent<game::Damage>(projectile, damage);

    const game::Entity areaAttack = session.registry.createEntity();
    session.registry.addComponent<game::PlayerTag>(areaAttack, {});
    session.registry.addComponent<game::Position>(areaAttack, {{700.0f, 400.0f}});
    session.registry.addComponent<view::Sprite>(areaAttack, {.rect = {{0.0f, 0.0f}, {83.0f, 40.0f}}});
    game::Damage damageArea = {
        .amount = 42,
        .pushBackForce = 0.0f,
        .stunChance = 0.0f,
        .kind = game::DamageKind::Area,
        .params =
            game::ProjectileDamage{
                .speed = 0.0f,
                .maxRange = 0.0f,
                .distanceTraveled = 0.0f,
                .maxTargets = 1,
            },
    };
    session.registry.addComponent<game::Damage>(areaAttack, damageArea);

    view::View view;
    game.updateView(view);

    unsigned orderI = 0;
    const std::array order = {areaAttack, enemy, boss, player, projectile};

    for (auto node : view.nodes) {
        auto p = std::get_if<std::reference_wrapper<const view::Sprite>>(&node.element);
        if (p && &p->get() == &session.registry.getComponent<view::Sprite>(order[orderI])) {
            orderI++;
        }
        if (orderI == order.size()) {
            break;
        }
    }
    REQUIRE(orderI == order.size());
}

TEST_CASE_METHOD(TestFixture, "Game updateView renders damaged enemy green and red health sections")
{
    game::Game game;
    game::GameDebugSession &session = game.getDebugSession();

    const game::Entity enemy = session.registry.createEntity();
    session.registry.addComponent<game::EnemyTag>(enemy, {});
    session.registry.addComponent<game::Position>(enemy, {{777.0f, 333.0f}});
    session.registry.addComponent<view::Sprite>(enemy, {.rect = {{0.0f, 0.0f}, {80.0f, 40.0f}}});

    game::EnemyStats stats;
    stats.maxHealth = 100.0f;
    stats.health = 50.0f;
    session.registry.addComponent<game::EnemyStats>(enemy, stats);
    session.registry.addComponent<game::HealthBarState>(
        enemy,
        {.previousHealth = 80.0f, .initialRedBarNorm = 0.25f, .redBarTimer = game::HealthBarState::redFlashDuration});

    view::View view;
    game.updateView(view);

    const auto rectangles = collectWorldRectangles(view);
    const float barX = 777.0f;
    const float barY = 333.0f - 10.0f - 5.0f;

    bool foundBackground = false;
    bool foundGreen = false;
    bool foundRed = false;

    for (const auto &rect : rectangles) {
        if (isColor(rect.fillColor, 0, 0, 0) && rect.rect.position.x == barX && rect.rect.position.y == barY) {
            foundBackground = true;
            REQUIRE(rect.rect.size.x == Catch::Approx(80.0f));
            REQUIRE(rect.rect.size.y == Catch::Approx(10.0f));
        }

        if (isColor(rect.fillColor, 0, 200, 0) && rect.rect.position.x == barX && rect.rect.position.y == barY) {
            foundGreen = true;
            REQUIRE(rect.rect.size.x == Catch::Approx(40.0f));
            REQUIRE(rect.rect.size.y == Catch::Approx(10.0f));
        }

        if (isColor(rect.fillColor, 220, 0, 0) && rect.rect.position.y == barY) {
            foundRed = true;
            REQUIRE(rect.rect.position.x == Catch::Approx(817.0f));
            REQUIRE(rect.rect.size.x == Catch::Approx(20.0f));
            REQUIRE(rect.rect.size.y == Catch::Approx(10.0f));
        }
    }

    REQUIRE(foundBackground);
    REQUIRE(foundGreen);
    REQUIRE(foundRed);
}

TEST_CASE_METHOD(TestFixture, "Game loadFromPersistedGame applies persisted values")
{
    game::PersistedGame persistedGame;
    persistedGame.wave = 5;
    persistedGame.playerStats.currency = 1234;
    persistedGame.playerStats.moveSpeed = 333.0f;

    game::Game game(persistedGame);

    const game::PersistedGame snapshot = game.getPersistedGame();
    REQUIRE(snapshot.wave == 5);
    int resultStage = ((snapshot.wave - 1) / controller::PersistenceManager::getConfig().wavesPerStage) + 1;
    REQUIRE(game.getDebugSession().stage == resultStage);
    REQUIRE(snapshot.playerStats.currency == 1234);
    REQUIRE(snapshot.playerStats.moveSpeed == 333.0f);
}