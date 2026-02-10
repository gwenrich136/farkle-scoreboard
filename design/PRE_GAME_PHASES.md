> **Scope:** Details the game phases that occur before the main gameplay loop starts (e.g., menu systems, player setup).
> **Status:** **LIVE DOCUMENT**

# Pre-Game Phases

## 1. Overview
Pre-game phases handle the initialization of the game before the first turn begins. This includes player selection and managing the transition into the main gameplay loop.

## 2. General Purpose
The goal of this category is to populate the `GameState` with the necessary information to start a match.

## 3. Phase Relationships
The system follows this sequence:
`PlayerSelectionPhase` -> `WaitingPhase` (In-Game)

## 4. Technical Details

### PreGamePhase
*   **Why:** Intermediate class for all pre-game states. It simplifies concrete implementations by providing a shared `display()` method that ensures the scoreboard's secondary displays (`ScoreDisplay`, `FarkleWarningLights`) remain OFF.
*   **Defined in:** `src/farkle/include/GamePhase.h`
*   **Implementation Details:**
    *   **Shared Display Logic**: Implements `GamePhase::display()` by explicitly clearing the `ScoreDisplay` and `FarkleWarningLights`.
    *   **Virtual Hooks**: Provides `updateProgressGrid()` and `updateTextDisplay()` hooks for concrete subclasses.

### PlayerSelectionPhase
*   **Why:** Allows users to choose players from a predefined list and populates the `GameState` before gameplay begins.
*   **Inherits From:** `PreGamePhase`
*   **Defined in:** `src/farkle/include/phases/PlayerSelectionPhase.h` & `src/farkle/src/phases/PlayerSelectionPhase.cpp`
*   **Implementation Details:**
    1.  **Name Pool**: Maintains a static list of available names: "Geewee", "Sammy", "Coach", "Sheshe", "Alex", "Tigre", "Pepa", "Fred", and "Andrea".
    2.  **Navigation**: 
        *   `UP_1000` / `DOWN_50`: Increments/decrements a selection index.
        *   The list of available names is filtered in real-time to exclude names already present in `state.players`.
    3.  **Adding Players (GREEN/BANK)**: 
        *   Checks `game.canAddPlayer()` (which queries `grid.isMaxPlayersReached()`).
        *   If allowed, calls `game.addPlayer(selectedName)`. This updates both the `GameState` and the hardware (color assignment in `LedProgressGrid`).
    4.  **Starting Game (RED/FARKLE)**: 
        *   If `state.players.size() >= 1`, returns `game.getPhase<WaitingPhase>()`.
    5.  **Display Behavior (Hooks)**:
        *   **`updateTextDisplay()`**: Calls `oled.printSelectionScreen("Add Player", currentSelection)`.
        *   **`updateProgressGrid()`**: Calls `grid.displayPlayersPregame(isPlayerPending)`. `isPlayerPending` is true if the grid is not full.
