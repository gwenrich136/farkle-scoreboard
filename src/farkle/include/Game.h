#ifndef Game_h
#define Game_h

#include "GameState.h"
#include "Displays.h"
#include "ButtonActions.h"
#include "GamePhase.h"
#include <type_traits>

#include "phases/WaitingPhase.h"
#include "phases/BankingPhase.h"
#include "phases/FarklingPhase.h"
#include "phases/PostGamePhase_V1.h"
#include "phases/PenaltyFarklePhase.h"

// Hardware Component Includes
#include "ControlPad.h"
#include "ScoreDisplay.h"
#include "LedProgressGrid.h"
#include "FarkleWarningLights.h"
#include "TextDisplay.h"

class Game {
public:
    Game();
    void setup();
    void loop();

    // Templated helper to get a pointer to a specific phase from the pool
    template<typename T>
    T* getPhase() {
        if (std::is_same<T, WaitingPhase>::value) return (T*)&phasePool.waiting;
        if (std::is_same<T, BankingPhase>::value) return (T*)&phasePool.banking;
        if (std::is_same<T, FarklingPhase>::value) return (T*)&phasePool.farkling;
        if (std::is_same<T, PostGamePhase_V1>::value) return (T*)&phasePool.postGame;
        if (std::is_same<T, PenaltyFarklePhase>::value) return (T*)&phasePool.penaltyFarkle;
        return nullptr;
    }

#ifdef UNIT_TEST
public:
#else
private:
#endif
    struct PhasePool {
        WaitingPhase waiting;
        BankingPhase banking;
        FarklingPhase farkling;
        PostGamePhase_V1 postGame;
        PenaltyFarklePhase penaltyFarkle;
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
    TextDisplay oled;
};

#endif
