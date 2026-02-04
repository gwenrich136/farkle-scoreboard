#ifndef PostGamePhase_V1_h
#define PostGamePhase_V1_h

#include "GamePhase.h"
#include <vector>
#include <string>

class PostGamePhase_V1 : public GamePhase {
public:
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) override;
    virtual void display(const GameState& state, const Displays& displays) override;

private:
    int m_winnerIdx;
    int m_highestScore;
    std::string m_winnerMsg;
    std::vector<int> m_scores;
};

#endif
