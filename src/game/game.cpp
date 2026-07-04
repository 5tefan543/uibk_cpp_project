#include "game/game.hpp"
#include "config/animation_config_helper.hpp"
#include "controller/debug/debug_context.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "controller/state/state_transition_action.hpp"
#include "controller/timing.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/camera_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/health_bar_state.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_attack_cooldown.hpp"
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
    playerStats.specialAttackSpeed = classConfig.stats.specialAttackSpeed;
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
      locationTable_(config_.locationTableConfig.numBuckets, config_.mapConfig.mapSize),
      shouldOpenStore_(persistedGame.shouldOpenStore)
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

    view::Sprite mapSprite = {.rect = {mapSpriteConfig.texture.position, mapSpriteConfig.texture.size},
                              .imagePath = mapSpriteConfig.texture.path};

    HitBox mapHitBox{mapSpriteConfig.hitBox.offset, mapSpriteConfig.hitBox.size};

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

    mapSprite.rect = {mapSpriteConfig.texture.position, mapSpriteConfig.texture.size};
    mapSprite.imagePath = mapSpriteConfig.texture.path;

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

    view::Sprite playerSprite{.rect =
                                  {
                                      {0, 0},
                                      animationFrame.spriteConfig.texture.size,
                                  },
                              .imagePath = animationFrame.spriteConfig.texture.path};

    HitBox hitBox{animationFrame.spriteConfig.hitBox.offset, animationFrame.spriteConfig.hitBox.size};

    registry_.addComponent<PlayerStats>(player, playerStats);
    registry_.addComponent<Position>(player, position);
    registry_.addComponent<Velocity>(player, {0.0f, 0.0f});
    registry_.addComponent<Animation>(player, playerAnimation);
    registry_.addComponent<view::Sprite>(player, playerSprite);
    registry_.addComponent<HitBox>(player, hitBox);
    registry_.addComponent<HealthBarState>(player, {});
    registry_.addComponent<PlayerAttackCooldown>(player, {});
}

void Game::initWave(int waveNumber)
{
    currentWaveDuration_ = std::chrono::seconds(0);
    wave_ = waveNumber;
    debugSession_.wave = waveNumber;

    int stageOld = stage_;
    stage_ = ((wave_ - 1) / config_.wavesPerStage) + 1;

    debugSession_.stage = stage_;

    if (wave_ > 1) {
        save();

        if (stage_ != stageOld) {
            switchMap();
        }
    }

    resetPlayerHealth();
    spawnEnemySystem_.update(registry_, wave_, config_);
    logger::log(logger::DEBUG, std::format("Starting wave {} of stage {}", wave_, stage_));
}

void Game::resetPlayerHealth()
{
    auto players = registry_.view<PlayerStats, PlayerTag>();
    if (!players.empty()) {
        PlayerStats &playerStats = registry_.getComponent<PlayerStats>(players.front());
        playerStats.health = playerStats.maxHealth;
    }
}

void Game::Game::save()
{
    PersistedGame persistedGame = getPersistedGame();
    if (!controller::PersistenceManager::saveGame(persistedGame)) {
        logger::log(logger::ERROR, "Failed to save game!");
        // TODO error via gui not console
    }
}

void Game::setShouldOpenStore(bool shouldOpenStore)
{
    shouldOpenStore_ = shouldOpenStore;
}

GameDebugSession &Game::getDebugSession()
{
    return debugSession_;
}

PersistedGame Game::getPersistedGame() const
{
    PersistedGame persistedGame;
    persistedGame.wave = wave_;
    persistedGame.shouldOpenStore = shouldOpenStore_;

    auto players = registry_.view<Position, PlayerStats, PlayerTag>();
    if (!players.empty()) {
        const Position &position = registry_.getComponent<Position>(players.front());
        persistedGame.position = position;
        const PlayerStats &playerStats = registry_.getComponent<PlayerStats>(players.front());
        persistedGame.playerStats = playerStats;
    }

    return persistedGame;
}

PlayerStats &Game::getPlayerStats()
{
    auto players = registry_.view<PlayerStats, PlayerTag>();
    if (!players.empty()) {
        return registry_.getComponent<PlayerStats>(players.front());
    }

    throw std::runtime_error("No player entity found when trying to get player stats");
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

controller::StateTransitionAction Game::update(const controller::InputState &input, const controller::timeDelta &dt)
{
    processDebugSession(dt);
    updateSystems(input, dt);

    // update clock
    currentWaveDuration_ += dt;

    if (shouldOpenStore_) {
        return controller::StateTransitionAction::PushProgressionStore;
    }

    if (isWaveFinished()) {
        cleanup();
        addScore(config_.waveDurationSeconds - (int)controller::toSeconds(currentWaveDuration_));

        shouldOpenStore_ = (wave_ % config_.wavesPerStage) == 0;
        initWave(++wave_);

        if (shouldOpenStore_) {
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

void Game::processDebugSession(const controller::timeDelta &dt)
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
        save();
    }
}

void Game::updateSystems(const controller::InputState &input, const controller::timeDelta &dt)
{
    controller::DebugContext &debug = controller::DebugContext::get();

    // Always update debug selection system
    debugSelectionSystem_.update(registry_, input, debug.active, debugSession_);

    if (debug.active && !debugSession_.isSystemUpdateActive) {
        return;
    }

    const float dtSec = controller::toSeconds(dt);
    locationTable_.update(registry_);
    enemyAI_.update(registry_, config_, locationTable_, dtSec);
    inputSystem_.update(registry_, config_, input, dtSec);
    movementSystem_.update(registry_, dtSec);
    animationSystem_.update(registry_, config_, dtSec);
    cameraSystem_.update(registry_);
    collisionDetectionSystem_.update(registry_);
    damageSystem_.update(registry_, dtSec);
    healthBarSystem_.update(registry_, dtSec);
    soundSystem_.update(registry_);
}

bool Game::isWaveFinished()
{
    bool isWaveTimeFinished = currentWaveDuration_ >= std::chrono::seconds(config_.waveDurationSeconds);
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

    if (shouldOpenStore_) {
        return;
    }

    setCameraPosition(view);
    renderSprites(view);
    renderHealthBars(view);
    renderDebugHitBoxes(view);
    renderStageWaveInfo(view);
    renderCooldownBars(view);
}

void Game::setCameraPosition(view::View &view)
{
    auto cameraEntities = registry_.view<CameraTag, Position>();
    if (!cameraEntities.empty()) {
        view.cameraPosition = registry_.getComponent<Position>(cameraEntities.front()).p;
    }
}

void Game::renderSprites(view::View &view)
{
    for (auto entity : registry_.view<Position, view::Sprite>()) {

        view::Sprite &sprite = registry_.getComponent<view::Sprite>(entity);
        sprite.rect.position = registry_.getComponent<Position>(entity).p;

        view.nodes.push_back({view::ViewMode::FixedToWorld, sprite});
    }
}

void game::Game::renderHealthBars(view::View &view)
{
    constexpr float barHeight = 10.0f;
    constexpr float barGap = 5.0f;
    constexpr float borderThickness = 2.0f;

    auto renderHealthBar = [&](Entity entity, const game::Stats &stats, const HealthBarState &bar) {
        const auto &pos = registry_.getComponent<Position>(entity).p;
        const auto &sprite = registry_.getComponent<view::Sprite>(entity);
        const float barWidth = sprite.rect.size.x;
        const float barX = pos.x;
        const float barY = pos.y - barHeight - barGap;

        // 1. Black background with green border (full bar width)
        view::Rectangle bgRect = {
            .rect = {{barX, barY}, {barWidth, barHeight}},
            .borderColor = view::color::mediumGreen,
            .thickness = borderThickness,
            .fillColor = view::color::black,
        };
        view.nodes.push_back({view::ViewMode::FixedToWorld, bgRect});

        // 2. Red flash section (right of the green portion)
        if (bar.redBarTimer > 0.0f) {
            const float greenWidth = (stats.health / stats.maxHealth) * barWidth;
            const float currentRedNorm = bar.initialRedBarNorm * (bar.redBarTimer / HealthBarState::redFlashDuration);
            const float redWidth = currentRedNorm * barWidth;
            if (redWidth > 0.0f) {
                view::Rectangle redRect = {
                    .rect = {{barX + greenWidth, barY}, {redWidth, barHeight}},
                    .borderColor = view::color::black,
                    .thickness = 0.0f,
                    .fillColor = view::color::strongRed,
                };
                view.nodes.push_back({view::ViewMode::FixedToWorld, redRect});
            }
        }

        // 3. Green current-health section
        const float greenWidth = (stats.health / stats.maxHealth) * barWidth;
        if (greenWidth > 0.0f) {
            view::Rectangle greenRect = {
                .rect = {{barX, barY}, {greenWidth, barHeight}},
                .borderColor = view::color::black,
                .thickness = 0.0f,
                .fillColor = view::color::mediumGreen,
            };
            view.nodes.push_back({view::ViewMode::FixedToWorld, greenRect});
        }
    };

    // Player — always show
    for (auto entity : registry_.view<PlayerTag, Position, view::Sprite, PlayerStats, HealthBarState>()) {
        const auto &stats = registry_.getComponent<PlayerStats>(entity);
        const auto &bar = registry_.getComponent<HealthBarState>(entity);
        renderHealthBar(entity, stats, bar);
    }

    // Enemies — only when not at full health
    for (auto entity : registry_.view<EnemyTag, Position, view::Sprite, EnemyStats, HealthBarState>()) {
        const auto &stats = registry_.getComponent<EnemyStats>(entity);
        if (stats.health >= stats.maxHealth) {
            continue;
        }
        const auto &bar = registry_.getComponent<HealthBarState>(entity);
        renderHealthBar(entity, stats, bar);
    }
}

void Game::renderDebugHitBoxes(view::View &view)
{
    controller::DebugContext &debug = controller::DebugContext::get();
    if (debug.active && debug.gameSettings.showHitboxes) {
        for (auto entity : registry_.view<Position, HitBox>()) {
            const auto &position = registry_.getComponent<Position>(entity).p;
            const HitBox &hitbox = registry_.getComponent<HitBox>(entity);

            view::Rectangle hitboxRect = {
                .rect = {position + hitbox.offset, hitbox.size},
                .borderColor = view::color::red,
                .thickness = 3.0f,
            };

            view.nodes.push_back({view::ViewMode::FixedToWorld, hitboxRect});
        }
    }
}

void Game::renderStageWaveInfo(view::View &view)
{
    stageWaveInfo_ = {
        .text = "Stage: " + std::to_string(stage_) + " Wave: " + std::to_string(wave_) + " Time remaining: "
                + std::to_string(config_.waveDurationSeconds
                                 - static_cast<int>(controller::toSeconds(currentWaveDuration_)))
                + " score: "
                + std::to_string(registry_.getComponent<PlayerStats>(registry_.view<PlayerTag>().front()).score)
                + " currency: "
                + std::to_string(registry_.getComponent<PlayerStats>(registry_.view<PlayerTag>().front()).currency),
        .size = 24,
        .position = {960.0f, 75.0f},
    };
    view.nodes.push_back({view::ViewMode::FixedToScreen, std::cref(stageWaveInfo_)});
}

void Game::renderCooldownBars(view::View &view)
{
    auto players = registry_.view<PlayerTag, PlayerStats, PlayerAttackCooldown>();
    if (!players.empty()) {
        const Entity player = players.front();
        const PlayerAttackCooldown &cooldown = registry_.getComponent<PlayerAttackCooldown>(player);
        const PlayerStats &playerStats = registry_.getComponent<PlayerStats>(player);

        constexpr float barWidth = 120.0f;
        constexpr float barHeight = 14.0f;
        constexpr float borderThickness = 2.0f;

        constexpr float startX = 15.0f;
        constexpr float startY = 60.0f;
        constexpr float gapBetween = barWidth + 100.0f;
        constexpr float iconBarGap = 10.0f;

        auto renderCooldown = [&](float x, float y, const view::Sprite &iconSprite, float remainingSec,
                                  float durationSec) {
            float progress = 1.0f;
            if (durationSec > 0.0f) {
                progress = std::clamp(1.0f - (remainingSec / durationSec), 0.0f, 1.0f);
            }

            const float fillWidth = barWidth * progress;

            view.nodes.push_back({view::ViewMode::FixedToScreen, iconSprite});

            const float barX = x + iconSprite.rect.size.x + iconBarGap;
            const float barY = y + (iconSprite.rect.size.y - barHeight) / 2.0f;

            view::Rectangle barBackground = {
                .rect = {{barX, barY}, {barWidth, barHeight}},
                .borderColor = view::color::white,
                .thickness = borderThickness,
                .fillColor = view::color::black,
            };

            view.nodes.push_back({view::ViewMode::FixedToScreen, barBackground});

            if (fillWidth > 0.0f) {
                view::Rectangle barFill = {
                    .rect = {{barX, barY}, {fillWidth, barHeight}},
                    .borderColor = view::color::black,
                    .thickness = 0.0f,
                    .fillColor = remainingSec <= 0.0f ? view::color::mediumGreen : view::color::brightBlue,
                };

                view.nodes.push_back({view::ViewMode::FixedToScreen, barFill});
            }
        };

        const config::TextureConfig &attackIconConfig =
            config_.playerClasses.getByType(playerStats.characterType).attack.attackIcon;

        const config::TextureConfig &specialAttackIconConfig =
            config_.playerClasses.getByType(playerStats.characterType).attack.specialAttackIcon;

        attackCooldownIcon_ = {
            .rect = {{startX, startY}, attackIconConfig.size},
            .imagePath = attackIconConfig.path,
        };

        specialAttackCooldownIcon_ = {
            .rect = {{startX + gapBetween, startY}, specialAttackIconConfig.size},
            .imagePath = specialAttackIconConfig.path,
        };

        renderCooldown(startX, startY, attackCooldownIcon_, cooldown.attackRemainingSec, cooldown.attackDurationSec);
        renderCooldown(startX + gapBetween, startY, specialAttackCooldownIcon_, cooldown.specialAttackRemainingSec,
                       cooldown.specialAttackDurationSec);
    }
}

} // namespace game