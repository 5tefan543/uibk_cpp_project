#pragma once

#include "game/ecs/component_storage.hpp"
#include "game/ecs/entity.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace game {

class Registry {
  private:
    Entity nextEntity = 1;
    std::vector<Entity> aliveEntities;
    std::vector<Entity> freeEntities;
    std::unordered_map<std::type_index, std::unique_ptr<IStorage>> storages;

    template <typename T>
    ComponentStorage<T> &getOrCreateStorage()
    {
        const std::type_index key(typeid(T));
        auto it = storages.find(key);

        if (it == storages.end()) {
            auto wrapper = std::make_unique<ComponentStorage<T>>();
            auto *rawPtr = wrapper.get();
            storages[key] = std::move(wrapper);
            return *rawPtr;
        }

        return *static_cast<ComponentStorage<T> *>(it->second.get());
    }

    template <typename T>
    const ComponentStorage<T> *tryGetStorage() const
    {
        const std::type_index key(typeid(T));
        auto it = storages.find(key);

        if (it == storages.end()) {
            return nullptr;
        }

        return static_cast<const ComponentStorage<T> *>(it->second.get());
    }

    bool isEntityAlive(Entity entity) const
    {
        return std::find(aliveEntities.begin(), aliveEntities.end(), entity) != aliveEntities.end();
    }

  public:
    Entity createEntity()
    {
        Entity entity;

        if (!freeEntities.empty()) {
            entity = freeEntities.back();
            freeEntities.pop_back();
        } else {
            entity = nextEntity++;
        }

        aliveEntities.push_back(entity);
        return entity;
    }

    void destroyEntity(Entity entity)
    {
        auto it = std::find(aliveEntities.begin(), aliveEntities.end(), entity);
        if (it == aliveEntities.end()) {
            return;
        }

        aliveEntities.erase(it);
        freeEntities.push_back(entity);

        for (auto &[type, storage] : storages) {
            storage->removeComponent(entity);
        }
    }

    const std::vector<Entity> &entities() const { return aliveEntities; }

    template <typename T>
    T &addComponent(Entity entity, T component)
    {
        if (!isEntityAlive(entity)) {
            throw std::runtime_error("Cannot add component to non-existing entity");
        }

        return getOrCreateStorage<T>().addComponent(entity, component);
    }

    template <typename T>
    void removeComponent(Entity entity)
    {
        getOrCreateStorage<T>().removeComponent(entity);
    }

    template <typename T>
    bool hasComponent(Entity entity) const
    {
        if (!isEntityAlive(entity)) {
            return false;
        }

        const auto *componentStorage = tryGetStorage<T>();
        return componentStorage != nullptr && componentStorage->hasComponent(entity);
    }

    template <typename T>
    T &getComponent(Entity entity)
    {
        return getOrCreateStorage<T>().getComponent(entity);
    }

    template <typename T>
    const T &getComponent(Entity entity) const
    {
        const auto *componentStorage = tryGetStorage<T>();
        if (componentStorage == nullptr) {
            throw std::runtime_error("Component storage does not exist");
        }

        return componentStorage->getComponent(entity);
    }

    template <typename... Components>
    std::vector<Entity> view() const
    {
        std::vector<Entity> matchingEntities;

        for (Entity entity : aliveEntities) {
            if ((hasComponent<Components>(entity) && ...)) {
                matchingEntities.push_back(entity);
            }
        }

        return matchingEntities;
    }
};

} // namespace game