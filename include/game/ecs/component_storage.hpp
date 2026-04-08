#pragma once

#include "game/ecs/entity.hpp"

#include <cstddef>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace game {

template <typename T>
class ComponentStorage {
  private:
    std::vector<Entity> entities;
    std::vector<T> components;
    std::unordered_map<Entity, std::size_t> entityToIndex;

  public:
    bool has(Entity entity) const { return entityToIndex.find(entity) != entityToIndex.end(); }

    template <typename... Args>
    T &emplace(Entity entity, Args &&...args)
    {
        if (has(entity)) {
            throw std::runtime_error("Component already exists on entity");
        }

        const std::size_t index = components.size();
        entities.push_back(entity);
        components.push_back(T{std::forward<Args>(args)...});
        entityToIndex[entity] = index;

        return components.back();
    }

    void remove(Entity entity)
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

    T &get(Entity entity)
    {
        auto it = entityToIndex.find(entity);
        if (it == entityToIndex.end()) {
            throw std::runtime_error("Component not found on entity");
        }

        return components[it->second];
    }

    const T &get(Entity entity) const
    {
        auto it = entityToIndex.find(entity);
        if (it == entityToIndex.end()) {
            throw std::runtime_error("Component not found on entity");
        }

        return components[it->second];
    }

    // const std::vector<Entity> &entities() const
    // {
    //     return entities;
    // }

    std::size_t size() const { return components.size(); }
};

} // namespace game