#ifndef PostGamePhase_V1_h
#define PostGamePhase_V1_h

#include "GamePhase.h"
#include <vector>
#include <string>

class PostGamePhase_V1 : public GamePhase {
public:
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) override;
    virtual void display(const GameState& state, const Displays& displays) override;

private:
    int m_winnerIdx;
    int m_highestScore;
    std::string m_winnerMsg;
    int m_scores[MAX_PLAYERS];
    int m_scoresCount = 0;
    bool m_finalized = false;
};

#endif
