> **Scope:** Details the game phases that occur before the main gameplay loop starts (e.g., menu systems, player setup).
> **Status:** **LIVE DOCUMENT**

# Pre-Game Phases

## 1. Overview
Pre-game phases handle the initialization of the game before the first turn begins. This includes player selection and managing the transition into the main gameplay loop.

## 2. General Purpose
The goal of this category is to populate the `GameState` with the necessary information to start a match.

## 3. Phase Relationships
The system follows this sequence:
`StartupPhase` -> `TargetScoreSelectionPhase` -> `PlayerSelectionPhase` -> `WaitingPhase` (In-Game)

## 4. Technical Details

### StartupPhase
*   **Why:** Serves as the absolute first entry point for the game upon boot. Determines whether there is a game to recover, and if so, allows the user to resume it or start a new game.
*   **Inherits From:** `PreGamePhase`
*   **Defined in:** `src/farkle/include/phases/StartupPhase.h` & `src/farkle/src/phases/StartupPhase.cpp`
*   **Implementation Details:**
    1.  **Selection Logic**: Checks `MemoryCard::hasActiveGame()`. If true, provides "Resume Game" (default) and "New Game" options. Otherwise, provides only "New Game".
    2.  **Navigation**:
        *   **Encoder Rotation**: Toggles between options (if active game exists).
    3.  **Confirmation (SELECT)**:
        *   If "Resume Game" is selected, calls `loadGameMetadata` and `replayGameJournal`. On success, calls `game.resumeGameDisplays()` to synchronize the hardware progress grid and score displays with the restored state, then transitions directly to `WaitingPhase`. On failure, clears active game state and falls back to `TargetScoreSelectionPhase`.
        *   If "New Game" is selected (or if no active game was present), clears any lingering active game state and transitions to `TargetScoreSelectionPhase`.
    4.  **Display Behavior (Hooks)**:
        *   **`updateTextDisplay()`**: Calls `textDisplay.printSelectionScreen("Farkle!", currentSelection)` where `currentSelection` is "Resume Game" or "New Game".
        *   **`updateProgressGrid()`**: The grid is already cleared via `Game::resetGame()`.

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
        *   **Encoder Rotation**: Increments or decrements `state.targetScore` in steps of 1,000.
    3.  **Confirmation (SELECT)**:
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
    1.  **Name Pool**: The list of available names is managed by the `MemoryCard` component, which reads from `players.csv` and sorts by play frequency.
    2.  **Navigation**: 
        *   **Encoder Rotation**: Increments or decrements the selection by calling `MemoryCard::getNextPlayer()` and `MemoryCard::getPreviousPlayer()`.
    3.  **Adding Players (SELECT)**:
        *   Checks `game.canAddPlayer()` (which queries `grid.isMaxPlayersReached()`).
        *   If allowed, calls `MemoryCard::reservePlayer()` to pull the name from the pool, then calls `game.addPlayer(reservedName)`. This updates both the `GameState` and the hardware (color assignment).
    4.  **Starting Game (RED/FARKLE)**: 
        *   If `state.players.size() >= 1`:
            *   Calls `MemoryCard::finalizeSelection()` to update and save the player pool.
            *   Generates a new Game ID (`getOrGenerateNextGameId`), sets it active, and creates the `/partial/[ID]/` directory.
            *   Writes the game setup (`targetScore` and player details) to `meta.jsn` via `MemoryCard::writeGameMetadata()`.
            *   Returns `game.getPhase<WaitingPhase>()`.
    5.  **Display Behavior (Hooks)**:
        *   **`updateTextDisplay()`**: Calls `textDisplay.printSelectionScreen("Add Player", currentSelection, getNextPlayerColor(state.players.size()))`. The `currentSelection` is rendered in the specific color that will be assigned to this player index, providing immediate visual feedback of their "identity" before they are added.
        *   **`updateProgressGrid()`**: Calls `grid.displayPlayersPregame(state.getNextPlayerHue(state.players.size()))`.
            *   **Color Sovereignty:** The `GameState` acts as the single source of truth for player colors. It generates unique hues using a Golden Ratio approximation (`(index * 40503) % 65536`).
            *   The phase queries the *prospective* hue for the next player from the `GameState` (using `std::optional` to safely manage the random seed for the first player) and passes it to the `LedProgressGrid` so the pending player's row can blink in their assigned color before confirmation.
