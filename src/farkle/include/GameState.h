#ifndef GameState_h
#define GameState_h

#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <Arduino.h>

#define MAX_SCORE 99999

struct Player {
    std::string name;
    int score;
    int farkle_count;
    uint16_t hue;

    Player(const std::string& n, uint16_t h = 0) : name(n), score(0), farkle_count(0), hue(h) {}
};

struct GameState {
    std::vector<Player> players;
    int atRiskScore;
    int currentPlayerIndex;
    bool finalRoundTriggered;
    int targetScore;

    uint32_t scoresVersion;

    mutable std::optional<uint16_t> prospectiveFirstHue;

    GameState() : atRiskScore(0), currentPlayerIndex(0), finalRoundTriggered(false), targetScore(10000), scoresVersion(1), prospectiveFirstHue(std::nullopt) {}

    void reset() {
        players.clear();
        atRiskScore = 0;
        currentPlayerIndex = 0;
        finalRoundTriggered = false;
        scoresVersion++;
        prospectiveFirstHue = std::nullopt;
    }

    uint16_t getNextPlayerHue(int playerCount) const {
        if (!prospectiveFirstHue.has_value()) {
            prospectiveFirstHue = random(0, 65536);
        }
        if (playerCount == 0) {
            return *prospectiveFirstHue;
        } else {
            return (*prospectiveFirstHue + (playerCount * 40503)) % 65536;
        }
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
