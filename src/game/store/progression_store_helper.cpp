#include "game/store/progression_store_helper.hpp"
#include <cmath>
#include <iomanip>
#include <numeric>
#include <sstream>

namespace game {

namespace {

constexpr float rarityBoostPerWave = 0.005f;
constexpr float maxRarityBoost = 0.5f;

float getRarityBoostForWave(std::size_t wave)
{
    if (wave <= 1) {
        return 0.0f;
    }

    // The rarity boost increases with each wave, but is capped at a maximum value to prevent excessive scaling.
    return std::min(maxRarityBoost, static_cast<float>(wave - 1) * rarityBoostPerWave);
}

std::vector<float> normalizeWeights(const std::vector<float> &weights)
{
    if (weights.empty()) {
        return weights;
    }

    std::vector<float> probabilities = weights;

    const float sum = std::accumulate(probabilities.begin(), probabilities.end(), 0.0f);

    if (sum <= 0.0f) {
        const float uniformProbability = 1.0f / static_cast<float>(probabilities.size());
        std::fill(probabilities.begin(), probabilities.end(), uniformProbability);
        return probabilities;
    }

    for (float &probability : probabilities) {
        probability /= sum;
    }

    return probabilities;
}

} // namespace

std::string floatToPrettyString(float value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << value;
    return stream.str();
}

geometry::Rectangle<float> scaleRectCentered(const geometry::Rectangle<float> &rect, float scale)
{
    const geometry::Vec2<float> center = rect.getCenter();
    const geometry::Vec2<float> scaledSize = rect.size * scale;

    return geometry::Rectangle<float>{.position = center - scaledSize / 2.0f, .size = scaledSize};
}

std::vector<float> getWaveAdjustedProbabilities(const std::vector<float> &baseWeights, std::size_t wave)
{
    if (baseWeights.empty()) {
        return {};
    }

    const float rarityBoost = getRarityBoostForWave(wave);

    std::vector<float> adjustedWeights;
    adjustedWeights.reserve(baseWeights.size());

    for (float baseWeight : baseWeights) {
        if (baseWeight <= 0.0f) {
            adjustedWeights.push_back(0.0f);
            continue;
        }

        const float adjustedWeight = std::pow(baseWeight, 1.0f - rarityBoost);
        adjustedWeights.push_back(adjustedWeight);
    }

    return normalizeWeights(adjustedWeights);
}
} // namespace game
