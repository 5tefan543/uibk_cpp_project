#pragma once

#include <cstdint>

namespace view {

struct Color {
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;
};

namespace color {

inline constexpr Color white = {255, 255, 255};
inline constexpr Color black = {0, 0, 0};

inline constexpr Color gray = {100, 100, 100};
inline constexpr Color lightGray = {150, 150, 150};
inline constexpr Color dimGray = {85, 85, 85};
inline constexpr Color darkGray = {65, 65, 65};
inline constexpr Color deepGray = {50, 50, 50};
inline constexpr Color veryDarkGray = {25, 25, 25};
inline constexpr Color nearBlack = {12, 12, 12};
inline constexpr Color almostBlack = {22, 22, 22};
inline constexpr Color charcoal = {28, 28, 28};

inline constexpr Color green = {0, 255, 0};
inline constexpr Color mediumGreen = {0, 200, 0};

inline constexpr Color brightBlue = {0, 120, 255};

inline constexpr Color red = {255, 0, 0};
inline constexpr Color brightRed = {250, 0, 10};
inline constexpr Color strongRed = {220, 0, 0};

inline constexpr Color darkMutedPurple = {55, 45, 55};
inline constexpr Color mutedPurple = {75, 60, 90};
inline constexpr Color lightMutedPurple = {90, 70, 110};

inline constexpr Color yellow = {245, 194, 17};

} // namespace color

} // namespace view