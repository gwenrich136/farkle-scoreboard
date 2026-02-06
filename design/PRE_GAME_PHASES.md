> **Scope:** Details the game phases that occur before the main gameplay loop starts (e.g., menu systems, player setup).
> **Status:** **LIVE DOCUMENT**

# Pre-Game Phases

## 1. Overview
Pre-game phases handle the initialization of the game before the first turn begins. This includes player count selection, name entry, and setting the target score.

## 2. General Purpose
The goal of this category is to populate the `GameState` with the necessary information to start a match.

## 3. Phase Relationships
Currently, the game skips explicit pre-game phases and initializes with a hardcoded 4-player setup. Future phases will likely follow a sequence:
`SplashScreen` -> `PlayerCountSelection` -> `PlayerNaming` -> `InGamePhase (Waiting)`

## 4. Technical Details
V1 does not implement any explicit pre-game phases. The `Game::setup()` method directly initializes the `GameState` and sets the initial phase to `WaitingPhase`.
