#pragma once

#include "button.hpp"
#include "sprite.hpp"
#include <memory>
#include <variant>

namespace controller {
// Forward declarations to avoid circular dependencies
struct Card; // Card used in ViewItem, but Card itself includes ViewItem, so we need a forward declaration here

// Readonly assembly of items to render
using ViewItem = std::variant<std::reference_wrapper<const Card>, std::reference_wrapper<const Button>,
                              std::reference_wrapper<const Text>,
                              Sprite // TODO: make Sprite reference_wrapped
                              >;

} // namespace controller