#include "game/game.hpp"
#include "controller/debug/debug_context.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/camera_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "view/sprite.hpp"
#include <iostream>
#include <view/text.hpp>

namespace game {

Game::Game(int wave, CharacterType characterType)
{
    config_ = controller::PersistenceManager::getConfig();
    initMap();
    initCamera();
    initPlayer(characterType);
    initWave(wave);
}

Game::Game() : Game(1, CharacterType::Melee)
{
    std::cout << "New game constructed" << std::endl;
}

Game::Game(CharacterType characterType) : Game(1, characterType)
{
    std::cout << "New game constructed" << std::endl;
}

Game::Game(const PersistedGame &persistedGame) : Game(persistedGame.wave, persistedGame.playerStats.characterType)
{
    std::cout << "Game constructed from persisted game" << std::endl;

    config_ = controller::PersistenceManager::getConfig();
    initMap();
    initCamera();
    initPlayer(persistedGame.position, persistedGame.playerStats);
    initWave(persistedGame.wave);
}

Game::~Game()
{
    std::cout << "Game destructed" << std::endl;
}

void Game::initMap()
{
    // Initialize map and camera
    Entity map = registry_.createEntity();
    registry_.addComponent<MapTag>(map, {});
    registry_.addComponent<Position>(map, {0.0f, 0.0f});
    view::Sprite mapSprite = {
        .imagePath = "assets/maps/map.bmp",
        .width = 1920.0f * 2.0f,
        .height = 1080.0f * 2.0f,
    };
    registry_.addComponent<view::Sprite>(map, mapSprite);
}

void Game::initCamera()
{
    Entity camera = registry_.createEntity();
    registry_.addComponent<CameraTag>(camera, {});
    registry_.addComponent<Position>(camera, {0.0f, 0.0f});
}

void Game::initPlayer(CharacterType characterType)
{
    Entity player = registry_.createEntity();
    registry_.addComponent<PlayerTag>(player, {});
    Animation playerAnimation;
    PlayerStats playerStats;
    playerStats.maxHealth = 100.0f;
    playerStats.health = playerStats.maxHealth;
    if (characterType == CharacterType::Melee) {
        playerStats.attackPower = 14.0f;
        playerStats.attackSpeed = 1.25f;
        playerStats.moveSpeed = 700.0f;
        playerStats.hasDash = true;
        playerStats.attackRange = 140.0f;
        playerStats.speedOfAttack = 220.0f;
        playerStats.dmgKind = DamageKind::MeleeArc;
        playerStats.characterType = CharacterType::Melee;
        playerAnimation = {.baseTexturePath = config_.assetConfig.meleeTexturePath};
        playerAnimation.attackTexturePath = config_.assetConfig.meleeTexturePath + "atk_";
        playerAnimation.attackMoveSpeedMultiplier = 0.5f;
    } else {
        playerStats.attackPower = 10.0f;
        playerStats.attackSpeed = 1.0f;
        playerStats.moveSpeed = 750.0f;
        playerStats.hasDash = false;
        playerStats.attackRange = 1000.0f;
        playerStats.speedOfAttack = 240.0f;
        playerStats.dmgKind = DamageKind::Projectile;
        playerStats.characterType = CharacterType::Ranged;
        playerAnimation = {.baseTexturePath = config_.assetConfig.rangedTexturePath};
    }
    playerStats.defense = 0.0f;
    registry_.addComponent<PlayerStats>(player, playerStats);

    registry_.addComponent<Position>(player, {100.0f, 100.0f});
    registry_.addComponent<Velocity>(player, {0.0f, 0.0f});
    registry_.addComponent<Animation>(player, playerAnimation);
    registry_.addComponent<view::Sprite>(player, {.imagePath = playerAnimation.baseTexturePath + "right_1.png"});
}

void Game::initPlayer(Position position, PlayerStats playerStats)
{
    Entity player = registry_.createEntity();
    registry_.addComponent<PlayerTag>(player, {});
    Animation playerAnimation;
    if (playerStats.characterType == CharacterType::Melee) {
        playerAnimation = {.baseTexturePath = config_.assetConfig.meleeTexturePath};
        playerAnimation.attackTexturePath = config_.assetConfig.meleeTexturePath + "atk_";
        playerAnimation.attackMoveSpeedMultiplier = 0.5f;
    } else {
        playerStats.dmgKind = DamageKind::Projectile;
        playerStats.characterType = CharacterType::Ranged;
        playerAnimation = {.baseTexturePath = config_.assetConfig.rangedTexturePath};
    }
    registry_.addComponent<PlayerStats>(player, playerStats);
    registry_.addComponent<Position>(player, position);
    registry_.addComponent<Velocity>(player, {0.0f, 0.0f});
    registry_.addComponent<Animation>(player, playerAnimation);
    registry_.addComponent<view::Sprite>(player, {.imagePath = playerAnimation.baseTexturePath + "right_1.png"});
}

void Game::initWave(int waveNumber)
{
    currentWaveDuration_ = 0.0f;
    wave_ = waveNumber;
    debugSession_.wave = waveNumber;

    stage_ = ((wave_ - 1) / config_.wavesPerStage) + 1;
    debugSession_.stage = stage_;

    if (wave_ > 1) {
        auto gameSave = getPersistedGame();
        if (!controller::PersistenceManager::saveGame(gameSave)) {

            // TODO error via gui not console
        }
    }

    spawnEnemySystem_.update(registry_, wave_, config_);

    std::cout << "Starting wave " << wave_ << " of stage " << stage_ << std::endl;
}

GameDebugSession &Game::getDebugSession()
{
    return debugSession_;
}

PersistedGame Game::getPersistedGame() const
{
    PersistedGame persistedGame;
    persistedGame.wave = wave_;

    auto players = registry_.view<Position, PlayerStats, PlayerTag>();
    if (!players.empty()) {
        const Position &position = registry_.getComponent<Position>(players.front());
        persistedGame.position = position;
        const PlayerStats &playerStats = registry_.getComponent<PlayerStats>(players.front());
        persistedGame.playerStats = playerStats;
    }

    return persistedGame;
}

controller::StateTransitionAction Game::update(const controller::InputState &input, float dt)
{
    processDebugSession(dt);
    updateSystems(input, dt);

    // update clock
    currentWaveDuration_ += dt;

    if (isWaveFinished()) {
        addScore(config_.waveDurationSeconds - (int)currentWaveDuration_);

        bool shouldOpenStore = (wave_ % config_.wavesPerStage) == 0;
        initWave(++wave_);

        if (shouldOpenStore) {
            return controller::StateTransitionAction::PushProgressionStore;
        }

        return controller::StateTransitionAction::None;
    }

    if (isGameOver()) {
        controller::PersistenceManager::deleteSave();
        return controller::StateTransitionAction::ReplaceCurrentWithGameOverMenu;
    }

    return controller::StateTransitionAction::None;
}

void Game::processDebugSession(float dt)
{
    controller::DebugContext &debug = controller::DebugContext::get();

    if (!debug.active) {
        return;
    }

    if (debugSession_.isClockPaused) {
        currentWaveDuration_ -= dt;
    }

    // Handle stage/wave reload request
    if (debugSession_.isStageWaveReloadRequested) {
        debugSession_.isStageWaveReloadRequested = false;
        initWave(debugSession_.wave);
    }

    // Handle player destruction request
    if (debugSession_.isPlayerDestructionRequested) {
        debugSession_.isPlayerDestructionRequested = false;
        std::cout << "Destroying player entity!" << std::endl;
        for (Entity player : registry_.view<PlayerTag>()) {
            registry_.destroyEntity(player);
        }
    }

    // Handle save game request
    if (debugSession_.isSaveGameRequested) {
        debugSession_.isSaveGameRequested = false;
        std::cout << "Saving game!" << std::endl;
        PersistedGame persistedGame = getPersistedGame();
        controller::PersistenceManager::saveGame(persistedGame);
    }
}

void Game::updateSystems(const controller::InputState &input, float dt)
{
    controller::DebugContext &debug = controller::DebugContext::get();

    // Always update debug selection system
    debugSelectionSystem_.update(registry_, input, debug.active, debugSession_);

    if (debug.active && !debugSession_.isSystemUpdateActive) {
        return;
    }

    inputSystem_.update(registry_, input, dt);
    movementSystem_.update(registry_, dt);
    animationSystem_.update(registry_, dt);
    cameraSystem_.update(registry_);
    collisionDetectionSystem_.update(registry_, wave_);
    damageSystem_.update(registry_, dt);
}

bool Game::isWaveFinished()
{
    bool isWaveTimeFinished = currentWaveDuration_ >= config_.waveDurationSeconds;
    bool isWaveDefeated = registry_.view<EnemyTag>().empty();

    return isWaveDefeated | isWaveTimeFinished;
}

bool Game::isGameOver()
{
    return registry_.view<PlayerTag>().empty();
}

void Game::addScore(int score)
{
    for (Entity player : registry_.view<PlayerTag>()) {
        PlayerStats &playerStats = registry_.getComponent<PlayerStats>(player);
        playerStats.score += score;
        playerStats.currency += score;
    }
}

void Game::updateView(view::View &view)
{
    // Game should be able to decide how to update.
    // Currently we simply clear everything and rebuilding the view from scratch.
    // Future improvements might only make changes and add/remove where necessary.
    view.nodes.clear();

    // Get camera data
    auto cameraEntities = registry_.view<CameraTag, Position>();
    if (!cameraEntities.empty()) {
        const Position &cameraPos = registry_.getComponent<Position>(cameraEntities.front());
        view.cameraX = cameraPos.x;
        view.cameraY = cameraPos.y;
    }

    // Render sprite entities
    for (auto entity : registry_.view<Position, view::Sprite>()) {

        view::Sprite &sprite = registry_.getComponent<view::Sprite>(entity);
        const Position &position = registry_.getComponent<Position>(entity);
        sprite.x = position.x;
        sprite.y = position.y;

        view.nodes.push_back({view::ViewMode::FixedToWorld, sprite});
    }

    controller::DebugContext &debug = controller::DebugContext::get();
    if (debug.active && debug.gameSettings.showHitboxes) {
        for (auto entity : registry_.view<Position, HitBox>()) {
            const HitBox &hitbox = registry_.getComponent<HitBox>(entity);
            view::Rectangle hitboxRect = {
                .width = hitbox.rect.width,
                .height = hitbox.rect.height,
                .gridX = hitbox.rect.x,
                .gridY = hitbox.rect.y,
                .borderColor = {255, 0, 0},
                .thickness = 6.0f,

            };

            view.nodes.push_back({view::ViewMode::FixedToWorld, hitboxRect});
        }
    }

    stageWaveInfo_ = {
        .text = "Stage: " + std::to_string(stage_) + " Wave: " + std::to_string(wave_) + " Time remaining: "
                + std::to_string(config_.waveDurationSeconds - static_cast<int>(currentWaveDuration_)) + " score: "
                + std::to_string(registry_.getComponent<PlayerStats>(registry_.view<PlayerTag>().front()).score)
                + " currency: "
                + std::to_string(registry_.getComponent<PlayerStats>(registry_.view<PlayerTag>().front()).currency),
        .size = 24,
        .gridX = 960.0f,
        .gridY = 75.0f,
    };
    view.nodes.push_back({view::ViewMode::FixedToScreen, std::cref(stageWaveInfo_)});
}
} // namespace game