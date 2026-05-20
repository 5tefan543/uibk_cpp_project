#pragma once

#include "button.hpp"
#include "rectangle.hpp"
#include "sprite.hpp"
#include <variant>

namespace view {
// Forward declarations to avoid circular dependencies
struct Card; // Card used in ViewElement, but Card itself includes ViewElement, so we need a forward declaration here

enum class ViewMode { FixedToScreen, FixedToWorld };

// Readonly assembly of elements to render
using ViewElement = std::variant<std::reference_wrapper<const Card>, std::reference_wrapper<const Button>,
                                 std::reference_wrapper<const Text>, std::reference_wrapper<const Rectangle>,
                                 Sprite // TODO: make Sprite reference_wrapped
                                 >;

struct ViewNode {
    ViewMode mode;
    ViewElement element;
};

// A ViewElement containing: gridX, gridY, width, height (could add sprite but currently not
// necessary)
template <typename T>
concept CenterableViewElement = requires(T t) { std::is_same_v<T, Button> || std::is_same_v<T, Card>; };

template <typename T>
    requires CenterableViewElement<T>
inline float getCenterX(const T &t)
{
    return t.gridX + t.width / 2;
}
template <typename T>
    requires CenterableViewElement<T>
inline float getCenterY(const T &t)
{
    return t.gridY + t.height / 2;
}

template <typename T>
    requires CenterableViewElement<T>
inline void setCenterizedX(T &t, float x)
{
    t.gridX = x - t.width / 2;
}

template <typename T>
    requires CenterableViewElement<T>
inline void setCenterizedY(T &t, float y)
{
    t.gridY = y - t.height / 2;
}

} // namespace view