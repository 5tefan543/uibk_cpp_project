#pragma once

#include "button.hpp"
#include "sprite.hpp"
#include <variant>

namespace controller {
// Forward declarations to avoid circular dependencies
struct Card; // Card used in ViewItem, but Card itself includes ViewItem, so we need a forward declaration here

// Readonly assembly of items to render
using ViewItem = std::variant<std::reference_wrapper<const Card>, std::reference_wrapper<const Button>,
                              std::reference_wrapper<const Text>,
                              Sprite // TODO: make Sprite reference_wrapped
                              >;

// A viewItem containing: gridX, gridY, width, height (could add sprite but currently not necessary)
template <typename T>
concept CenterableViewItem = requires(T t) { std::is_same_v<T, Button> || std::is_same_v<T, Card>; };

template <typename T>
    requires CenterableViewItem<T>
inline float getCenterX(const T &t)
{
    return t.gridX + t.width / 2;
}
template <typename T>
    requires CenterableViewItem<T>
inline float getCenterY(const T &t)
{
    return t.gridY + t.height / 2;
}

template <typename T>
    requires CenterableViewItem<T>
inline void setCenterizedX(T &t, float x)
{
    t.gridX = x - t.width / 2;
}

template <typename T>
    requires CenterableViewItem<T>
inline void setCenterizedY(T &t, float y)
{
    t.gridY = y - t.height / 2;
}

} // namespace controller