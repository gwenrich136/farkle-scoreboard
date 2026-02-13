> **Scope:** Details the game phases that occur before the main gameplay loop starts (e.g., menu systems, player setup).
> **Status:** **LIVE DOCUMENT**

# Pre-Game Phases

## 1. Overview
Pre-game phases handle the initialization of the game before the first turn begins. This includes player selection and managing the transition into the main gameplay loop.

## 2. General Purpose
The goal of this category is to populate the `GameState` with the necessary information to start a match.

## 3. Phase Relationships
The system follows this sequence:
`TargetScoreSelectionPhase` -> `PlayerSelectionPhase` -> `WaitingPhase` (In-Game)

## 4. Technical Details

### PreGamePhase
*   **Why:** Intermediate class for all pre-game states. It provides shared display behavior and ensures the scoreboard's secondary displays are used consistently during setup.
*   **Defined in:** `src/farkle/include/GamePhase.h` & `src/farkle/src/phases/PreGamePhase.cpp`
*   **Implementation Details:**
    *   **Shared Display Logic**: Implements `GamePhase::display()` by explicitly clearing the at-risk and current score segments as well as the Farkle warning lights. 
    *   **Goal Persistence**: It updates the `COMPETITION_SCORE` display to show `state.targetScore`, ensuring the goal is visible throughout the entire pre-game setup.
    *   **Virtual Hooks**: Provides `updateProgressGrid()` and `updateTextDisplay()` hooks for concrete subclasses.

### TargetScoreSelectionPhase
*   **Why:** Allows users to define the goal of the game before adding players.
*   **Inherits From:** `PreGamePhase`
*   **Defined in:** `src/farkle/include/phases/TargetScoreSelectionPhase.h` & `src/farkle/src/phases/TargetScoreSelectionPhase.cpp`
*   **Implementation Details:**
    1.  **Selection Logic**: Starts at a default of 10,000. Clamped between 1,000 and 20,000.
    2.  **Navigation**: 
        *   `UP_1000`: Increments `state.targetScore` by 1,000.
        *   `DOWN_50`: Decrements `state.targetScore` by 1,000.
    3.  **Confirmation (BANK or FARKLE)**: 
        *   Finalizes the selection by calling `game.setTargetScore(state.targetScore)` to sync hardware (LED Grid).
        *   Returns `game.getPhase<PlayerSelectionPhase>()`.
    4.  **Display Behavior (Hooks)**:
        *   **`updateTextDisplay()`**: Calls `oled.printSelectionScreen("Target Score", scoreString)`.
        *   **`updateProgressGrid()`**: Clears the grid.

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
