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
                                 std::reference_wrapper<const Text>, Rectangle, std::reference_wrapper<const Sprite>>;

struct ViewNode {
    ViewMode mode;
    ViewElement element;
};

} // namespace view