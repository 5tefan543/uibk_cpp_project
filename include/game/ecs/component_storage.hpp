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
    std::vector<Entity> entities_;
    std::vector<T> components_;
    std::unordered_map<Entity, std::size_t> entityToIndex_;

  public:
    T &addComponent(Entity entity, T component)
    {
        if (hasComponent(entity)) {
            throw std::runtime_error("Component already exists on entity");
        }

        const std::size_t index = components_.size();
        entities_.push_back(entity);
        components_.push_back(component);
        entityToIndex_[entity] = index;

        return components_.back();
    }

    void removeComponent(Entity entity) override
    {
        auto it = entityToIndex_.find(entity);
        if (it == entityToIndex_.end()) {
            return;
        }

        const std::size_t index = it->second;
        const std::size_t lastIndex = components_.size() - 1;

        if (index != lastIndex) {
            entities_[index] = entities_[lastIndex];
            components_[index] = std::move(components_[lastIndex]);
            entityToIndex_[entities_[index]] = index;
        }

        entities_.pop_back();
        components_.pop_back();
        entityToIndex_.erase(it);
    }

    bool hasComponent(Entity entity) const { return entityToIndex_.contains(entity); }

    T &getComponent(Entity entity)
    {
        auto it = entityToIndex_.find(entity);
        if (it == entityToIndex_.end()) {
            throw std::runtime_error("Component not found on entity");
        }

        return components_[it->second];
    }

    const T &getComponent(Entity entity) const
    {
        auto it = entityToIndex_.find(entity);
        if (it == entityToIndex_.end()) {
            throw std::runtime_error("Component not found on entity");
        }

        return components_[it->second];
    }
};

} // namespace game