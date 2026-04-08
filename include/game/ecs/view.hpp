#pragma once

#include "game/ecs/entity.hpp"

#include <cstddef>
#include <utility>
#include <vector>

namespace game {

template <typename RegistryType, typename... Components>
class View {
  private:
    RegistryType *registry;
    std::vector<Entity> entities;

  public:
    View(RegistryType &registry, std::vector<Entity> entities) : registry(&registry), entities(std::move(entities)) {}

    auto begin() { return entities.begin(); }

    auto end() { return entities.end(); }

    auto begin() const { return entities.begin(); }

    auto end() const { return entities.end(); }

    template <typename T>
    T &get(Entity entity)
    {
        return registry->template get<T>(entity);
    }

    template <typename T>
    const T &get(Entity entity) const
    {
        return registry->template get<T>(entity);
    }

    bool empty() const { return entities.empty(); }

    std::size_t size() const { return entities.size(); }
};

} // namespace game