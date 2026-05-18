#pragma once
#include "SFML/Graphics.hpp"

namespace game {
struct HitBox {
    sf::FloatRect rect;
    bool isActive;
};
}