#ifndef GameState_h
#define GameState_h

#include <string>
#include <vector>

struct Player {
    std::string name;
    int score;
    int farkle_count;
    std::vector<int> score_history;

    Player(const std::string& n) : name(n), score(0), farkle_count(0) {}
};

struct GameState {
    std::vector<Player> players;
    int atRiskScore;
    int currentPlayerIndex;
    bool finalRoundTriggered;
    int targetScore;

    GameState() : atRiskScore(0), currentPlayerIndex(0), finalRoundTriggered(false), targetScore(10000) {}
};

#endif
