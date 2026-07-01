#pragma once

#include "store_item.hpp"
#include <string>
#include <unordered_map>

namespace game {

struct PersistedStore {
    std::unordered_map<std::string, StoreItemType> nameToSelectedType;
};

} // namespace game
