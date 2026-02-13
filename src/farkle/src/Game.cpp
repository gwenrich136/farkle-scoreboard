#include "Game.h"
#include <Arduino.h>

Game::Game() : 
    controlPad(),
    scoreDisplay(12, 11, 10), // dataPin, clkPin, csPin
    grid(A0),                // NeoPixel Data Pin
    farkleLights(A1, A2),    // Yellow, Red pins
    oled(),
    currentPhase(nullptr),
    lastUpdateTime(0)
{
}

void Game::setup() {
    Serial.println("GAME: Initializing hardware...");
    
    // 1. Initialize Hardware
    Serial.println("GAME: Init OLED...");
    oled.begin();
    
    Serial.println("GAME: Init ScoreDisplay...");
    scoreDisplay.begin();
    scoreDisplay.addDisplay(ScoreDisplay::DisplayType::AT_RISK_SCORE, 0);
    scoreDisplay.addDisplay(ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE, 1);
    scoreDisplay.addDisplay(ScoreDisplay::DisplayType::COMPETITION_SCORE, 2);
    
    Serial.println("GAME: Init Grid...");
    grid.begin();
    
    Serial.println("GAME: Init FarkleLights...");
    farkleLights.begin();
    
    Serial.println("GAME: Hardware init done. Configuring controls...");
    // Configure ControlPad buttons as per SCHEMATIC_AND_HARDWARE_GUIDE.md
    // Note: D3=RIGHT_500 and D2=UP_1000 mapping is intentional.
    controlPad.addButton(4, DOWN_50);
    controlPad.addButton(5, LEFT_100);
    controlPad.addButton(3, RIGHT_500);
    controlPad.addButton(2, UP_1000);
    controlPad.addButton(6, BANK);
    controlPad.addButton(7, CLEAR);
    controlPad.addButton(8, FARKLE);

    // 2. Reset Game to clean state
    resetGame();

    // 3. Set Initial State
    currentPhase = &phasePool.playerSelection;
    currentPhase->onEnter(state);
    
    lastUpdateTime = millis();
}

void Game::loop() {
    // 1. Calculate deltaTime
    unsigned long currentTime = millis();
    unsigned long deltaTime = currentTime - lastUpdateTime;
    lastUpdateTime = currentTime;

    // 2. Read Input
    ButtonAction action = controlPad.read();

    // 3. Construct Displays struct
    Displays displays(scoreDisplay, grid, farkleLights, oled);

    // 4. Update Current Phase
    GamePhase* nextPhase = currentPhase->update(*this, state, action, deltaTime);

    // 5. Handle Transitions
    if (nextPhase != currentPhase) {
        currentPhase = nextPhase;
        currentPhase->onEnter(state);
    }

    // 6. Display Current State
    currentPhase->display(state, displays);
}

void Game::addPlayer(const std::string& name) {
    if (!canAddPlayer()) return;
    state.players.push_back(Player(name));
    grid.addPlayer();
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
