#pragma once

#include "game/ecs/entity.hpp"

#include <cstddef>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace game {

class IStorage {
  public:
    virtual ~IStorage() = default;
    virtual void removeComponent(Entity entity) = 0;
};

template <typename T>
class ComponentStorage : public IStorage {
  private:
    std::vector<Entity> entities;
    std::vector<T> components;
    std::unordered_map<Entity, std::size_t> entityToIndex;

  public:
    T &addComponent(Entity entity, T component)
    {
        if (hasComponent(entity)) {
            throw std::runtime_error("Component already exists on entity");
        }

        const std::size_t index = components.size();
        entities.push_back(entity);
        components.push_back(component);
        entityToIndex[entity] = index;

        return components.back();
    }

    void removeComponent(Entity entity) override
    {
        auto it = entityToIndex.find(entity);
        if (it == entityToIndex.end()) {
            return;
        }

        const std::size_t index = it->second;
        const std::size_t lastIndex = components.size() - 1;

        if (index != lastIndex) {
            entities[index] = entities[lastIndex];
            components[index] = std::move(components[lastIndex]);
            entityToIndex[entities[index]] = index;
        }

        entities.pop_back();
        components.pop_back();
        entityToIndex.erase(it);
    }

    bool hasComponent(Entity entity) const { return entityToIndex.contains(entity); }

    T &getComponent(Entity entity)
    {
        auto it = entityToIndex.find(entity);
        if (it == entityToIndex.end()) {
            throw std::runtime_error("Component not found on entity");
        }

        return components[it->second];
    }

    const T &getComponent(Entity entity) const
    {
        auto it = entityToIndex.find(entity);
        if (it == entityToIndex.end()) {
            throw std::runtime_error("Component not found on entity");
        }

        return components[it->second];
    }
};

} // namespace game