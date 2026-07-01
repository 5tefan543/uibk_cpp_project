#pragma once

namespace game {

enum StoreItemType { Common, Uncommon, Rare, Epic };

inline const char *toString(StoreItemType type)
{
    switch (type) {
    case StoreItemType::Common:
        return "Common";
    case StoreItemType::Uncommon:
        return "Uncommon";
    case StoreItemType::Rare:
        return "Rare";
    case StoreItemType::Epic:
        return "Epic";
    default:
        return "Unknown";
    }
};
} // namespace game
