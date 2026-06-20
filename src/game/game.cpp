#include "game/game.hpp"
#include "config/animation_config_helper.hpp"
#include "controller/debug/debug_context.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "controller/state/state_transition_action.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/camera_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "logging/log.hpp"
#include "view/sprite.hpp"
#include <format>
#include <view/text.hpp>

namespace game {

namespace {
void applyClassStats(const config::PlayerClassConfig &classConfig, PlayerStats &playerStats)
{
    playerStats.maxHealth = classConfig.stats.maxHealth;
    playerStats.health = playerStats.maxHealth;
    playerStats.attackPower = classConfig.stats.attackPower;
    playerStats.attackSpeed = classConfig.stats.attackSpeed;
    playerStats.defense = classConfig.stats.defense;
    playerStats.moveSpeed = classConfig.stats.moveSpeed;
    playerStats.speedOfAttack = classConfig.stats.speedOfAttack;
    playerStats.attackRange = classConfig.stats.attackRange;
    playerStats.hasDash = classConfig.hasDash;
    playerStats.characterType = classConfig.characterType;
}
} // namespace

Game::Game(int wave, CharacterType characterType)
    : config_(controller::PersistenceManager::getConfig()),
      locationTable_(config_.locationTableConfig.numBuckets, config_.mapConfig.mapSize)
{
    initMap();
    initCamera({0.0f, 0.0f});
    initPlayer(characterType);
    initWave(wave);
}

Game::Game() : Game(1, CharacterType::Melee)
{
    logger::log(logger::DEBUG, "New game constructed");
}

Game::Game(CharacterType characterType) : Game(1, characterType)
{
    logger::log(logger::DEBUG, "New game constructed");
}

Game::Game(const PersistedGame &persistedGame)
    : config_(controller::PersistenceManager::getConfig()),
      locationTable_(config_.locationTableConfig.numBuckets, config_.mapConfig.mapSize)
{
    logger::log(logger::DEBUG, "Game constructed from persisted game");

    initMap();
    initCamera(persistedGame.position);
    initPlayer(persistedGame.position, persistedGame.playerStats);
    initWave(persistedGame.wave);
}

Game::~Game()
{
    logger::log(logger::DEBUG, "Game destructed");
}

void Game::initMap()
{
    int mapIdx = (stage_ - 1) % config_.mapConfig.mapSprites.size();
    auto &mapSpriteConfig = config_.mapConfig.mapSprites[mapIdx];

    view::Sprite mapSprite = {
        .x = mapSpriteConfig.texture.position.x,
        .y = mapSpriteConfig.texture.position.y,
        .imagePath = mapSpriteConfig.texture.path,
        .width = mapSpriteConfig.texture.size.x,
        .height = mapSpriteConfig.texture.size.y,
    };

    HitBox mapHitBox{.offset = mapSpriteConfig.hitBox.offset, .size = mapSpriteConfig.hitBox.size};

    Entity map = registry_.createEntity();
    registry_.addComponent<MapTag>(map, {});
    registry_.addComponent<Position>(map, {0.0f, 0.0f});
    registry_.addComponent<view::Sprite>(map, mapSprite);
    registry_.addComponent<HitBox>(map, mapHitBox);
}

void Game::switchMap()
{
    auto mapEntities = registry_.view<MapTag, Position, view::Sprite>();
    if (mapEntities.empty()) {
        logger::log(logger::ERROR, "No map entity found when trying to switch map");
        return;
    }

    Entity mapEntity = mapEntities.front();
    view::Sprite &mapSprite = registry_.getComponent<view::Sprite>(mapEntity);
    HitBox &mapHitBox = registry_.getComponent<HitBox>(mapEntity);

    int mapIdx = (stage_ - 1) % config_.mapConfig.mapSprites.size();
    auto &mapSpriteConfig = config_.mapConfig.mapSprites[mapIdx];

    mapSprite.x = mapSpriteConfig.texture.position.x;
    mapSprite.y = mapSpriteConfig.texture.position.y;
    mapSprite.imagePath = mapSpriteConfig.texture.path;
    mapSprite.width = mapSpriteConfig.texture.size.x;
    mapSprite.height = mapSpriteConfig.texture.size.y;

    mapHitBox.offset = mapSpriteConfig.hitBox.offset;
    mapHitBox.size = mapSpriteConfig.hitBox.size;
}

void Game::initCamera(Position position)
{
    Entity camera = registry_.createEntity();
    registry_.addComponent<CameraTag>(camera, {});
    registry_.addComponent<Position>(camera, position);
}

void Game::initPlayer(CharacterType characterType)
{
    Position position = {100.0f, 100.0f};
    PlayerStats playerStats;

    const config::PlayerClassConfig &classConfig = config_.playerClasses.getByType(characterType);
    applyClassStats(classConfig, playerStats);

    initPlayer(position, playerStats);
}

void Game::initPlayer(Position position, PlayerStats playerStats)
{
    Entity player = registry_.createEntity();
    registry_.addComponent<PlayerTag>(player, {});

    Animation playerAnimation{
        .state = AnimationState::Idle,
        .direction = AnimationDirection::Right,
    };

    const config::AnimationFrame animationFrame = config::AnimationConfigHelper::getPlayerAnimationFrame(
        config_, playerStats.characterType, playerAnimation.state, playerAnimation.direction,
        playerAnimation.currentFrame);

    view::Sprite playerSprite{.imagePath = animationFrame.spriteConfig.texture.path,
                              .width = animationFrame.spriteConfig.texture.size.x,
                              .height = animationFrame.spriteConfig.texture.size.y};

    HitBox hitBox{.offset = animationFrame.spriteConfig.hitBox.offset, .size = animationFrame.spriteConfig.hitBox.size};

    registry_.addComponent<PlayerStats>(player, playerStats);
    registry_.addComponent<Position>(player, position);
    registry_.addComponent<Velocity>(player, {0.0f, 0.0f});
    registry_.addComponent<Animation>(player, playerAnimation);
    registry_.addComponent<view::Sprite>(player, playerSprite);
    registry_.addComponent<HitBox>(player, hitBox);
}

void Game::initWave(int waveNumber)
{
    currentWaveDuration_ = 0.0f;
    wave_ = waveNumber;
    debugSession_.wave = waveNumber;

    int stageOld = stage_;
    stage_ = ((wave_ - 1) / config_.wavesPerStage) + 1;

    debugSession_.stage = stage_;

    if (wave_ > 1) {
        auto gameSave = getPersistedGame();

        if (!controller::PersistenceManager::saveGame(gameSave)) {

            // TODO error via gui not console
        }

        if (stage_ != stageOld) {
            switchMap();
        }
    }

    spawnEnemySystem_.update(registry_, wave_, config_);
    logger::log(logger::DEBUG, std::format("Starting wave {} of stage {}", wave_, stage_));
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
void Game::cleanup()
{
    std::vector<Entity> enemyEntities = registry_.view<EnemyTag>();
    for (Entity enemy : enemyEntities) {
        registry_.destroyEntity(enemy);
    }
    std::vector<Entity> damageEntities = registry_.view<Damage>();
    for (Entity damage : damageEntities) {
        registry_.destroyEntity(damage);
    }
}

controller::StateTransitionAction Game::update(const controller::InputState &input, float dt)
{
    processDebugSession(dt);
    updateSystems(input, dt);

    // update clock
    currentWaveDuration_ += dt;

    if (isWaveFinished()) {
        cleanup();
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
        logger::log(logger::DEBUG, "Destroying player entity!");

        for (Entity player : registry_.view<PlayerTag>()) {
            registry_.destroyEntity(player);
        }
    }

    // Handle save game request
    if (debugSession_.isSaveGameRequested) {
        debugSession_.isSaveGameRequested = false;
        logger::log(logger::DEBUG, "Saving game!");
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
    logger::log(logger::DEBUG, "update game!");

    locationTable_.update(registry_);
    enemyAI_.update(registry_, config_, locationTable_, dt);
    inputSystem_.update(registry_, config_, input, dt);
    movementSystem_.update(registry_, dt);
    animationSystem_.update(registry_, config_, dt);
    cameraSystem_.update(registry_);
    collisionDetectionSystem_.update(registry_);
    damageSystem_.update(registry_, dt);
    soundSystem_.update(registry_);
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

int Game::getWaveNumber()
{
    return wave_;
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
            const Position &position = registry_.getComponent<Position>(entity);
            const HitBox &hitbox = registry_.getComponent<HitBox>(entity);

            view::Rectangle hitboxRect = {
                .width = hitbox.size.x,
                .height = hitbox.size.y,
                .gridX = position.x + hitbox.offset.x,
                .gridY = position.y + hitbox.offset.y,
                .borderColor = {255, 0, 0},
                .thickness = 3.0f,
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