#include "controller/input/input_state.hpp"
#include "view/button.hpp"
#include <deque>
#include <optional>

namespace controller {

struct MouseUtil {

    static std::optional<std::size_t> getHoveredButtonId(const InputState &input,
                                                         const std::deque<view::Button> &buttons)
    {
        for (std::size_t idx = 0; idx < buttons.size(); idx++) {
            if (buttons[idx].rect.contains(input.mouseGrid)) {
                return idx;
            }
        }

        return std::nullopt;
    }
};
} // namespace controller