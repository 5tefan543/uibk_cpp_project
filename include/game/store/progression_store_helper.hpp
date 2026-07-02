#pragma once

#include "geometry/rectangle.hpp"
#include <vector>

namespace game {

std::string floatToPrettyString(float value);
geometry::Rectangle<float> scaleRectCentered(const geometry::Rectangle<float> &rect, float scale);

// Moves the distribution toward equal probabilities as waves increase.
// The rarityBoost must stay below 1.0 so rarer item types become more likely,
// but common item types remain more likely than rare item types.
std::vector<float> getWaveAdjustedProbabilities(const std::vector<float> &baseWeights, std::size_t wave);

} // namespace game
