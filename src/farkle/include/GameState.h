#ifndef GameState_h
#define GameState_h

#include <string>
#include <vector>
#include <cstdint>

#define MAX_SCORE 99999

struct Player {
    std::string name;
    int score;
    int farkle_count;

    Player(const std::string& n) : name(n), score(0), farkle_count(0) {}
};

struct GameState {
    std::vector<Player> players;
    int atRiskScore;
    int currentPlayerIndex;
    bool finalRoundTriggered;
    int targetScore;

    uint32_t scoresVersion;

    GameState() : atRiskScore(0), currentPlayerIndex(0), finalRoundTriggered(false), targetScore(10000), scoresVersion(1) {}

    void reset() {
        players.clear();
        atRiskScore = 0;
        currentPlayerIndex = 0;
        finalRoundTriggered = false;
        scoresVersion++;
    }

    void updatePlayerScore(int playerIndex, int newScore) {
        if (playerIndex >= 0 && playerIndex < (int)players.size()) {
            if (players[playerIndex].score != newScore) {
                players[playerIndex].score = newScore;
                scoresVersion++;
            }
        }
    }

    void addPlayerScore(int playerIndex, int delta) {
        if (playerIndex >= 0 && playerIndex < (int)players.size()) {
            if (delta != 0) {
                players[playerIndex].score += delta;
                scoresVersion++;
            }
        }
    }
};

#endif
