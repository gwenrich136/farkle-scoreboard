> **Scope:** Details the game phases that occur after a game has concluded.
> **Status:** **LIVE DOCUMENT**

# Post-Game Phases

## 1. Overview
Post-game phases handle the display of final results and provide options to restart or return to the main menu.

## 2. General Purpose
The goal of this category is to celebrate the winner and show the final state of the game.

## 3. Phase Relationships
The game enters this category from `InGamePhase` when the win condition is met (see `WaitingPhase` for transition logic).
In V1, once entered, the game remains in `PostGamePhase_V1` indefinitely.

## 4. Technical Details

### PostGamePhase_V1
*   **Why:** Implements the simplified "frozen" post-game phase for V1.
*   **Defined in:** `src/farkle/include/phases/PostGamePhase_V1.h` & `src/farkle/src/phases/PostGamePhase_V1.cpp`
*   **Implementation Details:** Its `update()` method is empty and simply `return this;`, ignoring all input.
