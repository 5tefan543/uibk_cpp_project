#pragma once
#include <string>
#include <vector>
#include <array>
namespace controller {

    
    struct LeaderboardEntry {
        std::string playerName;
        int score;
        int wave = 0;
        
        bool operator<(const LeaderboardEntry &other) const
        {
            if (wave != other.wave) {
                return wave > other.wave;
        }
        if (score != other.score) {
            return score > other.score;
        }
        return playerName < other.playerName;
    }
    
    bool operator==(const LeaderboardEntry &other) const = default;
};
struct Leaderboard {
    std::vector<LeaderboardEntry> entries;
};

} // namespace controller