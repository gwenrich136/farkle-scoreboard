#ifndef PlayerSelectionPhase_h
#define PlayerSelectionPhase_h

#include "GamePhase.h"
#include <vector>
#include <string>

class PlayerSelectionPhase : public GamePhase {
public:
    PlayerSelectionPhase();
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) override;
    virtual void display(const GameState& state, const Displays& displays) override;

private:
    int m_selectionIndex;
    std::vector<std::string> m_namePool;
    std::vector<std::string> getFilteredNames(const GameState& state);
};

#endif
