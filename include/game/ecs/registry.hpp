#pragma once

#include "game/ecs/component_storage.hpp"
#include "game/ecs/entity.hpp"
#include "game/ecs/view.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace game {

class IStorage {
  public:
    virtual ~IStorage() = default;
    virtual void removeEntity(Entity entity) = 0;
};

template <typename T>
class StorageWrapper : public IStorage {
  public:
    ComponentStorage<T> storage;

    void removeEntity(Entity entity) override { storage.remove(entity); }
};

class Registry {
  private:
    Entity nextEntity = 1;
    std::vector<Entity> aliveEntities;
    std::unordered_map<std::type_index, std::unique_ptr<IStorage>> storages;

    template <typename T>
    ComponentStorage<T> &storage()
    {
        const std::type_index key(typeid(T));
        auto it = storages.find(key);

        if (it == storages.end()) {
            auto wrapper = std::make_unique<StorageWrapper<T>>();
            auto *rawPtr = wrapper.get();
            storages[key] = std::move(wrapper);
            return rawPtr->storage;
        }

        return static_cast<StorageWrapper<T> *>(it->second.get())->storage;
    }

    template <typename T>
    const ComponentStorage<T> *tryStorage() const
    {
        const std::type_index key(typeid(T));
        auto it = storages.find(key);

        if (it == storages.end()) {
            return nullptr;
        }

        return &static_cast<const StorageWrapper<T> *>(it->second.get())->storage;
    }

    bool isAlive(Entity entity) const
    {
        return std::find(aliveEntities.begin(), aliveEntities.end(), entity) != aliveEntities.end();
    }

  public:
    Entity create()
    {
        const Entity entity = nextEntity++;
        aliveEntities.push_back(entity);
        return entity;
    }

    void destroy(Entity entity)
    {
        auto it = std::find(aliveEntities.begin(), aliveEntities.end(), entity);
        if (it == aliveEntities.end()) {
            return;
        }

        aliveEntities.erase(it);

        for (auto &[type, storage] : storages) {
            (void)type;
            storage->removeEntity(entity);
        }
    }

    const std::vector<Entity> &entities() const { return aliveEntities; }

    template <typename T, typename... Args>
    T &emplace(Entity entity, Args &&...args)
    {
        if (!isAlive(entity)) {
            throw std::runtime_error("Cannot add component to non-existing entity");
        }

        return storage<T>().emplace(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    void remove(Entity entity)
    {
        storage<T>().remove(entity);
    }

    template <typename T>
    bool has(Entity entity) const
    {
        if (!isAlive(entity)) {
            return false;
        }

        const auto *componentStorage = tryStorage<T>();
        return componentStorage != nullptr && componentStorage->has(entity);
    }

    template <typename T>
    T &get(Entity entity)
    {
        return storage<T>().get(entity);
    }

    template <typename T>
    const T &get(Entity entity) const
    {
        const auto *componentStorage = tryStorage<T>();
        if (componentStorage == nullptr) {
            throw std::runtime_error("Component storage does not exist");
        }

        return componentStorage->get(entity);
    }

    template <typename... Components>
    View<Registry, Components...> view()
    {
        std::vector<Entity> matchingEntities;

        for (Entity entity : aliveEntities) {
            if ((has<Components>(entity) && ...)) {
                matchingEntities.push_back(entity);
            }
        }

        return View<Registry, Components...>(*this, std::move(matchingEntities));
    }
};

} // namespace game