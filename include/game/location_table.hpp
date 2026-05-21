#pragma once
#include "game/ecs/entity.hpp"
#include "game/ecs/registry.hpp"
#include <unordered_set>

namespace game {
#define TESTING
#ifdef TESTING
struct LocationTableTester;
#endif

// Lookup 2D Table for finding entities based on their location in the world grid with dimensions view::gridWidth X
// view::gridHeight.
struct LocationTable {
    void update(const Registry &registry);
    std::unordered_set<Entity> getEntitiesNear(const float x, const float y, const float radius);
    LocationTable(const unsigned numBuckW, const unsigned numBuckH);
#ifdef TESTING
    friend LocationTableTester;
#endif

  private:
    const unsigned buckWidth;
    const unsigned buckHeight;
    const unsigned numBuckW;
    const unsigned numBuckH;
    std::vector<std::unique_ptr<std::vector<Entity>>> bucketGrid_;
    const std::vector<Entity> *cgetBucket(const unsigned buckIx, const unsigned buckIy) const
    {
        return bucketGrid_[buckIy * numBuckW + buckIx].get();
    }
    std::vector<Entity> *getBucket(const unsigned buckIx, const unsigned buckIy)
    {
        return bucketGrid_[buckIy * numBuckW + buckIx].get();
    }
};

#ifdef TESTING
struct LocationTableTester {
    const LocationTable &lt;
    LocationTableTester(LocationTable &lt) : lt(lt) {};
    auto getBuckWidth() { return lt.buckWidth; }
    auto getBuckHeight() { return lt.buckHeight; }
    auto getNumBuckW() { return lt.numBuckW; }
    auto getNumBuckH() { return lt.numBuckH; }
    auto getBucket(unsigned buckIx, unsigned buckIy) { return lt.cgetBucket(buckIx, buckIy); }
    const auto &getGrid() { return lt.bucketGrid_; }
};
#endif

} // namespace game