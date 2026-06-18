#ifndef StartupPhase_h
#define StartupPhase_h

#include "GamePhase.h"

enum class StartupOption {
    RESUME_GAME,
    NEW_GAME
};

class StartupPhase : public PreGamePhase {
public:
    StartupPhase() : _selectionIndex(0) {}
    virtual void onEnter(Game& game, GameState& state) override;
    virtual GamePhase* update(Game& game, GameState& state, GameInput input, unsigned long deltaTime) override;

protected:
    virtual void updateTextDisplay(const GameState& state, const Displays& displays) override;

private:
    int _selectionIndex;
    std::vector<StartupOption> _options;

    StartupOption getSelectedOption() const;
    void populateOptions(Game& game);

    GamePhase* launchResumeGame(Game& game, GameState& state);
    GamePhase* launchStartNewGame(Game& game, GameState& state);
};

#endif
