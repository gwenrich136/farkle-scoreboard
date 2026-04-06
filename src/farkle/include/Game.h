#ifndef Game_h
#define Game_h

#include "GameState.h"
#include "Displays.h"
#include "Input.h"
#include "GamePhase.h"
#include <type_traits>

#include "phases/TargetScoreSelectionPhase.h"
#include "phases/PlayerSelectionPhase.h"
#include "phases/WaitingPhase.h"
#include "phases/BankingPhase.h"
#include "phases/FarklingPhase.h"
#include "phases/PenaltyFarklingPhase.h"
#include "phases/PostGamePhase_V1.h"

// Hardware Component Includes
#include "ControlPad.h"
#include "ScoreDisplay.h"
#include "LedProgressGrid.h"
#include "FarkleWarningLights.h"
#include "TextDisplayV2.h"

class Game {
public:
    Game();
    void setup();
    void loop();

    // Templated helper to get a pointer to a specific phase from the pool
    template<typename T>
    T* getPhase() {
        if (std::is_same<T, TargetScoreSelectionPhase>::value) return (T*)&phasePool.targetScoreSelection;
        if (std::is_same<T, PlayerSelectionPhase>::value) return (T*)&phasePool.playerSelection;
        if (std::is_same<T, WaitingPhase>::value) return (T*)&phasePool.waiting;
        if (std::is_same<T, BankingPhase>::value) return (T*)&phasePool.banking;
        if (std::is_same<T, FarklingPhase>::value) return (T*)&phasePool.farkling;
        if (std::is_same<T, PenaltyFarklingPhase>::value) return (T*)&phasePool.penaltyFarkling;
        if (std::is_same<T, PostGamePhase_V1>::value) return (T*)&phasePool.postGame;
        return nullptr;
    }

    void addPlayer(const std::string& name);
    bool canAddPlayer();
    void resetGame();
    void setTargetScore(int target);

#ifdef UNIT_TEST
public:
#else
private:
#endif
    struct PhasePool {
        TargetScoreSelectionPhase targetScoreSelection;
        PlayerSelectionPhase playerSelection;
        WaitingPhase waiting;
        BankingPhase banking;
        FarklingPhase farkling;
        PenaltyFarklingPhase penaltyFarkling;
        PostGamePhase_V1 postGame;
    };

    PhasePool phasePool;
    GameState state;
    GamePhase* currentPhase;
    unsigned long lastUpdateTime;

    // Hardware Components
    ControlPad controlPad;
    ScoreDisplay scoreDisplay;
    LedProgressGrid grid;
    FarkleWarningLights farkleLights;
    TextDisplayV2 textDisplay;
};

#endif
