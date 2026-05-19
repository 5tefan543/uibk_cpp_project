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
            const view::Button &button = buttons[idx];
            const bool insideX = input.mouseGridX >= button.gridX && input.mouseGridX <= (button.gridX + button.width);
            const bool insideY = input.mouseGridY >= button.gridY && input.mouseGridY <= (button.gridY + button.height);

            if (insideX && insideY) {
                return idx;
            }
        }

        return std::nullopt;
    }
};
} // namespace controller