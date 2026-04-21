#include "Game.h"
#include <Arduino.h>

Game::Game() : 
    controlPad(),
    scoreDisplay(10),        // csPin (Hardware SPI uses D11 for MOSI, D13 for SCK)
    grid(A0),                // NeoPixel Data Pin
    farkleLights(A1),        // Status Strip NeoPixel Pin
    textDisplay(A4, A5, D7, D8), // cs, dc, res, blk
    memoryCard(9),           // SD Card CS Pin
    currentPhase(nullptr),
    lastUpdateTime(0)
{
}

void Game::setup() {
    Serial.println("GAME: Initializing hardware...");
    
    // 1. Initialize Hardware
    Serial.println("GAME: Init ControlPad...");
    controlPad.begin();

    Serial.println("GAME: Init TextDisplayV2...");
    textDisplay.begin();
    
    Serial.println("GAME: Init ScoreDisplay...");
    scoreDisplay.begin();
    scoreDisplay.addDisplay(ScoreDisplay::DisplayType::AT_RISK_SCORE, 0);
    scoreDisplay.addDisplay(ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE, 1);
    scoreDisplay.addDisplay(ScoreDisplay::DisplayType::COMPETITION_SCORE, 2);
    
    Serial.println("GAME: Init Grid...");
    grid.begin();
    
    Serial.println("GAME: Init FarkleLights...");
    farkleLights.begin();
    
    Serial.println("GAME: Init MemoryCard...");
    memoryCard.begin();

    // 2. Reset Game to clean state
    resetGame();

    // 3. Set Initial State
    currentPhase = &phasePool.targetScoreSelection;
    currentPhase->onEnter(state);
    
    lastUpdateTime = millis();
}

void Game::loop() {
    // 1. Calculate deltaTime
    unsigned long currentTime = millis();
    unsigned long deltaTime = currentTime - lastUpdateTime;
    lastUpdateTime = currentTime;

    // 2. Read Input
    GameInput input = controlPad.read();
    state.currentPlayerScoreMode = input.scoreDisplayMode;

    // 3. Construct Displays struct
    Displays displays(scoreDisplay, grid, farkleLights, textDisplay);

    // 4. Update Current Phase
    GamePhase* nextPhase = currentPhase->update(*this, state, input, deltaTime);

    // 5. Handle Transitions
    if (nextPhase != currentPhase) {
        currentPhase = nextPhase;
        currentPhase->onEnter(state);
    }

    // 6. Display Current State
    currentPhase->display(state, displays);
}

void Game::addPlayer(const char* name) {
    if (!canAddPlayer()) return;
    uint16_t hue = state.getNextPlayerHue(state.players.size());
    state.players.push_back(Player(name, hue));
    grid.addPlayer(hue);
}

bool Game::canAddPlayer() {
    return !grid.isMaxPlayersReached();
}

void Game::resetGame() {
    state.reset();
    grid.reset();
    farkleLights.farkle_state(0);
    scoreDisplay.clear(ScoreDisplay::DisplayType::AT_RISK_SCORE);
    scoreDisplay.clear(ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE);
    scoreDisplay.clear(ScoreDisplay::DisplayType::COMPETITION_SCORE);
}

void Game::setTargetScore(int target) {
    state.targetScore = target;
    grid.setTargetScore(target);
    scoreDisplay.print_number(target, ScoreDisplay::DisplayType::COMPETITION_SCORE);
}
