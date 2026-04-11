#pragma once

#include "button.hpp"
#include <memory>
#include <variant>

namespace controller {
// Forward declarations to avoid circular dependencies
struct Card; // Card used in ViewItem, but Card itself includes ViewItem, so we need a forward declaration here

using ViewItem = std::variant<std::unique_ptr<Card>, Button, Text>;
} // namespace controller