#ifndef PlayerSelectionPhase_h
#define PlayerSelectionPhase_h

#include "GamePhase.h"
#include <vector>
#include <string>

class PlayerSelectionPhase : public PreGamePhase {
public:
    virtual ~PlayerSelectionPhase() = default;
    virtual void onEnter(GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) override;

protected:
    virtual void updateProgressGrid(const GameState& state, const Displays& displays) override;
    virtual void updateTextDisplay(const GameState& state, const Displays& displays) override;

private:
    int m_selectionIndex;
    static const std::vector<std::string> s_namePool;

    std::vector<std::string> getAvailableNames(const GameState& state) const;
};

#endif
