#ifndef Game_h
#define Game_h

#include "GameState.h"
#include "Displays.h"
#include "Input.h"
#include "GamePhase.h"
#include <type_traits>

#include "phases/StartupPhase.h"
#include "phases/TargetScoreSelectionPhase.h"
#include "phases/PlayerSelectionPhase.h"
#include "phases/WaitingPhase.h"
#include "phases/BankingPhase.h"
#include "phases/FarklingPhase.h"
#include "phases/PenaltyFarklingPhase.h"
#include "phases/PostGamePhase_V1.h"
#include "phases/EndOfTurnPhase.h"

// Hardware Component Includes
#include "ControlPad.h"
#include "ScoreDisplay.h"
#include "LedProgressGrid.h"
#include "FarkleWarningLights.h"
#include "TextDisplayV2.h"
#include "MemoryCard.h"
#include "SoundPlayer.h"

class Game {
public:
    Game();
    void setup();
    void loop();

    // Templated helper to get a pointer to a specific phase from the pool
    template<typename T>
    T* getPhase() {
        if (std::is_same<T, StartupPhase>::value) return (T*)&phasePool.startup;
        if (std::is_same<T, TargetScoreSelectionPhase>::value) return (T*)&phasePool.targetScoreSelection;
        if (std::is_same<T, PlayerSelectionPhase>::value) return (T*)&phasePool.playerSelection;
        if (std::is_same<T, WaitingPhase>::value) return (T*)&phasePool.waiting;
        if (std::is_same<T, BankingPhase>::value) return (T*)&phasePool.banking;
        if (std::is_same<T, FarklingPhase>::value) return (T*)&phasePool.farkling;
        if (std::is_same<T, PenaltyFarklingPhase>::value) return (T*)&phasePool.penaltyFarkling;
        if (std::is_same<T, PostGamePhase_V1>::value) return (T*)&phasePool.postGame;
        if (std::is_same<T, EndOfTurnPhase>::value) return (T*)&phasePool.endOfTurn;
        return nullptr;
    }

    void addPlayer(const char* name);
    bool canAddPlayer();
    void resetGame();
    void resumeGameDisplays();
    void setTargetScore(int target);
    MemoryCard& getMemoryCard() { return memoryCard; }
    SoundPlayer& getSoundPlayer() { return soundPlayer; }

#ifdef UNIT_TEST
public:
#else
private:
#endif
    struct PhasePool {
        StartupPhase startup;
        TargetScoreSelectionPhase targetScoreSelection;
        PlayerSelectionPhase playerSelection;
        WaitingPhase waiting;
        BankingPhase banking;
        FarklingPhase farkling;
        PenaltyFarklingPhase penaltyFarkling;
        PostGamePhase_V1 postGame;
        EndOfTurnPhase endOfTurn;
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
    MemoryCard memoryCard;
    SoundPlayer soundPlayer;
};

#endif
