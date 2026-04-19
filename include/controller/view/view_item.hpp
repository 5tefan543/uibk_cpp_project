#pragma once

#include "button.hpp"
#include <memory>
#include <variant>

namespace controller {
// Forward declarations to avoid circular dependencies
struct Card; // Card used in ViewItem, but Card itself includes ViewItem, so we need a forward declaration here

// Button shared with MenuState buttons_ list
using ViewItem = std::variant<std::unique_ptr<Card>, std::shared_ptr<Button>, Text>;
} // namespace controller