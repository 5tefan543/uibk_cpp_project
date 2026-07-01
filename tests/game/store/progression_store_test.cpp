#include "controller/persistence/persistence_manager.hpp"
#include "game/store/progression_store.hpp"
#include "game/store/progression_store_helper.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {

config::StoreItemTypeConfig makeTypeConfig(int cost, const std::string &iconPath, const game::PlayerStats &statChanges)
{
    config::StoreItemTypeConfig typeConfig;
    typeConfig.cost = cost;
    typeConfig.icon.path = iconPath;
    typeConfig.statChanges = statChanges;
    return typeConfig;
}

config::StoreItemConfig makeStoreItemConfig(const std::string &name, const std::string &description,
                                            game::StoreItemType type, const config::StoreItemTypeConfig &typeConfig)
{
    config::StoreItemConfig itemConfig;
    itemConfig.name = name;
    itemConfig.description = description;
    itemConfig.typeToConfig.emplace(type, typeConfig);
    return itemConfig;
}

config::GameConfig createStoreTestConfig()
{
    config::GameConfig config = controller::PersistenceManager::getConfig();

    config.storeConfig.typeToRandomWeight = {
        {game::StoreItemType::Common, 1.0f},
        {game::StoreItemType::Uncommon, 1.0f},
        {game::StoreItemType::Rare, 1.0f},
        {game::StoreItemType::Epic, 1.0f},
    };

    game::PlayerStats attackChanges = game::getDefaultPlayerStatChanges();
    attackChanges.attackPower = 5.0f;

    game::PlayerStats defenseChanges = game::getDefaultPlayerStatChanges();
    defenseChanges.defense = 3.0f;

    config.storeConfig.items = {
        makeStoreItemConfig("Attack Upgrade", "Increases attack", game::StoreItemType::Common,
                            makeTypeConfig(100, "attack-icon.png", attackChanges)),
        makeStoreItemConfig("Defense Upgrade", "Increases defense", game::StoreItemType::Rare,
                            makeTypeConfig(200, "defense-icon.png", defenseChanges)),
    };

    REQUIRE(controller::PersistenceManager::saveConfig(config));

    return config;
}

const view::Card &getBackgroundCard(const view::View &view)
{
    REQUIRE(view.nodes.size() == 1);
    return ViewElementAccessor::as<const view::Card>(view.nodes[0].element);
}

geometry::Vec2<float> getStoreItemCenter(const view::View &view, std::size_t itemIndex)
{
    const view::Card &backgroundCard = getBackgroundCard(view);
    REQUIRE(backgroundCard.elements.size() >= 4);
    const view::Card &itemsCard = ViewElementAccessor::as<const view::Card>(backgroundCard.elements[3]);

    REQUIRE(itemIndex + 1 < itemsCard.elements.size());
    const view::Card &itemCard = ViewElementAccessor::as<const view::Card>(itemsCard.elements[itemIndex + 1]);
    return itemCard.rect.getCenter();
}

geometry::Vec2<float> getBuyButtonCenter(const view::View &view)
{
    const view::Card &backgroundCard = getBackgroundCard(view);
    REQUIRE(backgroundCard.elements.size() >= 5);
    const view::Card &detailsCard = ViewElementAccessor::as<const view::Card>(backgroundCard.elements[4]);

    REQUIRE(detailsCard.elements.size() >= 7);
    const view::Button &buyButton = ViewElementAccessor::as<const view::Button>(detailsCard.elements[6]);
    return buyButton.rect.getCenter();
}

controller::InputState mouseClickAt(const geometry::Vec2<float> &position)
{
    controller::InputState input;
    input.mouseMoved = true;
    input.mouseLeftPressed = true;
    input.mouseGrid = position;
    return input;
}

} // namespace

TEST_CASE_METHOD(TestFixture, "ProgressionStore loads items from config and persists selected item types")
{
    controller::PersistenceManager::deleteStore();
    createStoreTestConfig();

    game::Game game;
    game::ProgressionStore store(game);

    view::View view;
    store.initView(view);

    const std::optional<game::PersistedStore> persistedStore = controller::PersistenceManager::getStore();

    REQUIRE(persistedStore.has_value());
    REQUIRE(persistedStore->nameToSelectedType.size() == 2);
    REQUIRE(persistedStore->nameToSelectedType.at("Attack Upgrade") == game::StoreItemType::Common);
    REQUIRE(persistedStore->nameToSelectedType.at("Defense Upgrade") == game::StoreItemType::Rare);
}

TEST_CASE_METHOD(TestFixture, "ProgressionStore selects store item with mouse click")
{
    controller::PersistenceManager::deleteStore();
    createStoreTestConfig();

    game::Game game;
    game::ProgressionStore store(game);

    view::View view;
    store.initView(view);

    const controller::InputState clickStoreItemInput = mouseClickAt(getStoreItemCenter(view, 1));

    REQUIRE_FALSE(store.selectedStoreItemChanged());
    REQUIRE(store.update(clickStoreItemInput) == controller::StateTransitionAction::None);
    REQUIRE(store.selectedStoreItemChanged());
}

TEST_CASE_METHOD(TestFixture, "ProgressionStore buying selected item applies stat changes and subtracts currency")
{
    controller::PersistenceManager::deleteStore();
    createStoreTestConfig();

    game::Game game;
    game.getPlayerStats().currency = 150;
    game.getPlayerStats().attackPower = 10.0f;
    game.getPlayerStats().defense = 2.0f;

    game::ProgressionStore store(game);

    view::View view;
    store.initView(view);

    const controller::InputState clickBuyButtonInput = mouseClickAt(getBuyButtonCenter(view));

    REQUIRE(store.update(clickBuyButtonInput) == controller::StateTransitionAction::None);
    REQUIRE(store.buyButtonPressed());
    REQUIRE(game.getPlayerStats().currency == 50);
    REQUIRE(game.getPlayerStats().attackPower == Catch::Approx(15.0f));
    REQUIRE(game.getPlayerStats().defense == Catch::Approx(2.0f));
}

TEST_CASE_METHOD(TestFixture, "ProgressionStore buying mouse-selected item applies selected item stat changes")
{
    controller::PersistenceManager::deleteStore();
    createStoreTestConfig();

    game::Game game;
    game.getPlayerStats().currency = 300;
    game.getPlayerStats().attackPower = 10.0f;
    game.getPlayerStats().defense = 2.0f;

    game::ProgressionStore store(game);

    view::View view;
    store.initView(view);

    const controller::InputState clickStoreItemInput = mouseClickAt(getStoreItemCenter(view, 1));
    const controller::InputState clickBuyButtonInput = mouseClickAt(getBuyButtonCenter(view));

    REQUIRE(store.update(clickStoreItemInput) == controller::StateTransitionAction::None);
    REQUIRE(store.selectedStoreItemChanged());

    REQUIRE(store.update(clickBuyButtonInput) == controller::StateTransitionAction::None);
    REQUIRE(store.buyButtonPressed());
    REQUIRE(game.getPlayerStats().currency == 100);
    REQUIRE(game.getPlayerStats().attackPower == Catch::Approx(10.0f));
    REQUIRE(game.getPlayerStats().defense == Catch::Approx(5.0f));
}

TEST_CASE_METHOD(TestFixture, "ProgressionStore does not buy selected item when currency is too low")
{
    controller::PersistenceManager::deleteStore();
    createStoreTestConfig();

    game::Game game;
    game.getPlayerStats().currency = 99;
    game.getPlayerStats().attackPower = 10.0f;
    game.getPlayerStats().defense = 2.0f;

    game::ProgressionStore store(game);

    view::View view;
    store.initView(view);

    const controller::InputState clickBuyButtonInput = mouseClickAt(getBuyButtonCenter(view));

    REQUIRE(store.update(clickBuyButtonInput) == controller::StateTransitionAction::None);
    REQUIRE_FALSE(store.buyButtonPressed());
    REQUIRE(game.getPlayerStats().currency == 99);
    REQUIRE(game.getPlayerStats().attackPower == Catch::Approx(10.0f));
    REQUIRE(game.getPlayerStats().defense == Catch::Approx(2.0f));
}

TEST_CASE_METHOD(TestFixture, "ProgressionStore uses persisted store item type")
{
    controller::PersistenceManager::deleteStore();

    config::GameConfig config = controller::PersistenceManager::getConfig();
    config.storeConfig.typeToRandomWeight = {
        {game::StoreItemType::Common, 0.5f},
        {game::StoreItemType::Uncommon, 0.3f},
        {game::StoreItemType::Rare, 0.15f},
        {game::StoreItemType::Epic, 0.05f},
    };

    game::PlayerStats attackChanges = game::getDefaultPlayerStatChanges();
    attackChanges.attackPower = 50.0f;

    config.storeConfig.items = {makeStoreItemConfig("Attack Upgrade", "Increases attack", game::StoreItemType::Epic,
                                                    makeTypeConfig(500, "attack-icon.png", attackChanges))};

    REQUIRE(controller::PersistenceManager::saveConfig(config));

    game::PersistedStore persistedStore;
    persistedStore.nameToSelectedType["Attack Upgrade"] = game::StoreItemType::Epic;
    REQUIRE(controller::PersistenceManager::saveStore(persistedStore));

    game::Game game;
    game.getPlayerStats().currency = 600;
    game.getPlayerStats().attackPower = 10.0f;

    game::ProgressionStore store(game);

    view::View view;
    store.initView(view);

    const controller::InputState clickBuyButtonInput = mouseClickAt(getBuyButtonCenter(view));

    REQUIRE(store.update(clickBuyButtonInput) == controller::StateTransitionAction::None);
    REQUIRE(store.buyButtonPressed());
    REQUIRE(game.getPlayerStats().currency == 100);
    REQUIRE(game.getPlayerStats().attackPower == Catch::Approx(60.0f));
}

TEST_CASE_METHOD(TestFixture, "Store item probabilities match config weights at wave one")
{
    const std::vector<float> baseWeights = {
        0.5f,  // Common
        0.3f,  // Uncommon
        0.15f, // Rare
        0.05f, // Epic
    };

    std::size_t wave = 1;
    const std::vector<float> probabilities = game::getWaveAdjustedProbabilities(baseWeights, wave);

    REQUIRE(probabilities.size() == 4);
    REQUIRE(probabilities[0] == Catch::Approx(0.5f));
    REQUIRE(probabilities[1] == Catch::Approx(0.3f));
    REQUIRE(probabilities[2] == Catch::Approx(0.15f));
    REQUIRE(probabilities[3] == Catch::Approx(0.05f));
}

TEST_CASE_METHOD(TestFixture, "Store item probabilities increase for rarer item types with increasing waves")
{
    const std::vector<float> baseWeights = {
        0.5f,  // Common
        0.3f,  // Uncommon
        0.15f, // Rare
        0.05f, // Epic
    };

    std::size_t wave = 1;
    const std::vector<float> waveOneProbabilities = game::getWaveAdjustedProbabilities(baseWeights, wave);

    wave = 100;
    const std::vector<float> laterWaveProbabilities = game::getWaveAdjustedProbabilities(baseWeights, wave);

    REQUIRE(laterWaveProbabilities.size() == 4);
    REQUIRE(laterWaveProbabilities[0] < waveOneProbabilities[0]); // Common decreases
    REQUIRE(laterWaveProbabilities[1] < waveOneProbabilities[1]); // Uncommon decreases
    REQUIRE(laterWaveProbabilities[2] > waveOneProbabilities[2]); // Rare increases
    REQUIRE(laterWaveProbabilities[3] > waveOneProbabilities[3]); // Epic increases

    const float laterWaveSum = std::accumulate(laterWaveProbabilities.begin(), laterWaveProbabilities.end(), 0.0f);
    REQUIRE(laterWaveSum == Catch::Approx(1.0f));
}