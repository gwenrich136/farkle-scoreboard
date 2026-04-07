#ifndef GameState_h
#define GameState_h

#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <Arduino.h>

#define MAX_SCORE 99999
#define HUE_MAX_16BIT 65536
#define HUE_GOLDEN_RATIO_JUMP 40503

struct Player {
    std::string name;
    int score;
    int farkle_count;
    uint16_t hue;

    Player(const std::string& n, uint16_t h = 0) : name(n), score(0), farkle_count(0), hue(h) {}
};

#include "Input.h"

struct GameState {
    std::vector<Player> players;
    int atRiskScore;
    int currentPlayerIndex;
    bool finalRoundTriggered;
    int targetScore;

    uint32_t scoresVersion;

    mutable std::optional<uint16_t> prospectiveFirstHue;

    std::vector<int> rankedPlayerIndices;
    int currentCompetitorRank;
    ScoreDisplayMode currentPlayerScoreMode;

    GameState() : atRiskScore(0), currentPlayerIndex(0), finalRoundTriggered(false), targetScore(10000), scoresVersion(1), prospectiveFirstHue(std::nullopt), currentCompetitorRank(0), currentPlayerScoreMode(ScoreDisplayMode::BANKED) {}

    void reset() {
        players.clear();
        atRiskScore = 0;
        currentPlayerIndex = 0;
        finalRoundTriggered = false;
        scoresVersion++;
        prospectiveFirstHue = std::nullopt;
        rankedPlayerIndices.clear();
        currentCompetitorRank = 0;
        currentPlayerScoreMode = ScoreDisplayMode::BANKED;
    }

    uint16_t getNextPlayerHue(int playerCount) const {
        if (!prospectiveFirstHue.has_value()) {
            prospectiveFirstHue = random(0, HUE_MAX_16BIT);
        }
        if (playerCount == 0) {
            return *prospectiveFirstHue;
        } else {
            return (*prospectiveFirstHue + (playerCount * HUE_GOLDEN_RATIO_JUMP)) % HUE_MAX_16BIT;
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
