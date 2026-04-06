> **Scope:** Details the game phases that make up the core gameplay loop.
> **Status:** **LIVE DOCUMENT**

# In-Game Phases

## 1. Overview
In-game phases manage the active gameplay where players take turns, roll dice, and bank points.

## 2. General Purpose
This category handles user input for scoring, provides feedback through animations, and manages turn transitions.

## 3. Phase Relationships
- `WaitingPhase` is the main hub.
- `WaitingPhase` -> `BankingPhase` (on Bank button)
- `WaitingPhase` -> `FarklingPhase` (on Farkle button)
- `WaitingPhase` -> `PenaltyFarklingPhase` (on 3rd consecutive Farkle)
- All animation phases (`BankingPhase`, `FarklingPhase`, `PenaltyFarklingPhase`) return to `EndOfTurnPhase` after animation completion.
- `EndOfTurnPhase` returns to `WaitingPhase` after a button press.
- `WaitingPhase` transitions to `PostGamePhase_V1` when a player reaches the target score and the final round completes.

## 4. Technical Details

### WaitingPhase
*   **Why:** Implements the main interactive phase and serves as the entry point for a player's turn.
*   **Defined in:** `src/farkle/include/phases/WaitingPhase.h` & `src/farkle/src/phases/WaitingPhase.cpp`
*   **Implementation Details:** The `update()` method contains all logic for this phase:
    *   **Check for Game End:** At the start of the `update` method, it checks if `state.finalRoundTriggered` is true. If it is, and the current player's score is `>= state.targetScore`, it immediately `return game.getPhase<PostGamePhase_V1>();`.
    *   **Handle Score Input:** A `switch(input.action)` block handles `PLUS_50`, `PLUS_100`, and `PLUS_500` by modifying `state.atRiskScore`.
    *   **Handle Navigation (Competitor Preview):** Uses `input.rotationDelta` (Encoder) to cycle through other players' scores on the `COMPETITION_SCORE` display.
    *   **Handle Transitions:** If `BANK` is pressed, `return game.getPhase<BankingPhase>();`. If `FARKLE` is pressed, it checks the current player's `farkle_count`. If the count is 2 or more, it `return game.getPhase<PenaltyFarklingPhase>();`. Otherwise, it `return game.getPhase<FarklingPhase>();`.

### BankingPhase
*   **Why:** Implements the banking animation and the end-of-turn logic that follows.
*   **Defined in:** `src/farkle/include/phases/BankingPhase.h` & `src/farkle/src/phases/BankingPhase.cpp`
*   **Implementation Details:**
    0.  **Reset Farkle Count:** The `onEnter()` method resets the current player's `farkle_count` to 0.
    1.  **Animate Score Transfer:** While `state.atRiskScore > 0`, run the time-based animation logic using `deltaTime` to incrementally move points from `state.atRiskScore` to the current player's banked score. Ignore all input during this stage.
    2.  **Transition:** Once `state.atRiskScore == 0`, the animation is complete and returns `game.getPhase<EndOfTurnPhase>()`.

### FarklingPhase
*   **Why:** Implements the farkle animation and the end-of-turn logic.
*   **Defined in:** `src/farkle/include/phases/FarklingPhase.h` & `src/farkle/src/phases/FarklingPhase.cpp`
*   **Implementation Details:**
    1.  **Increment Count:** The `onEnter()` method checks if `player.score > 0`. If true, it increments the `farkle_count`. If false (score is 0), the count is **not** incremented ("No Harm, No Foul").
    2.  **Animate Score Loss:** The `update()` method runs a time-based animation to drain `state.atRiskScore` to 0. It ignores input during the animation.
    3.  **Transition:** Once `state.atRiskScore == 0`, the animation is complete and returns `game.getPhase<EndOfTurnPhase>()`.

### PenaltyFarklingPhase
*   **Why:** Implements the catastrophic farkle penalty for a player's third consecutive farkle.
*   **Defined in:** `src/farkle/include/phases/PenaltyFarklingPhase.h` & `src/farkle/src/phases/PenaltyFarklingPhase.cpp`
*   **Implementation Details:**
    1.  **Initialize & Reset:** The `onEnter()` method:
        *   Calculates penalty: `min(1000, player.score)`.
        *   Sets `atRiskScore` to negative penalty (e.g., -1000).
        *   Resets the player's `farkle_count` to 0 immediately.
        *   Initializes a phase-local timer/state for the animation sequence.
    2.  **3-Stage Animation (`update()`):**
        *   **Stage 1: The Pain (0s - 5s):** No score changes. `atRisk` display is set to blink/flash. Warning lights alternate.
        *   **Stage 2: The Drain:** `atRisk` display stops blinking. Values animate: `atRisk` goes up to 0, Banked score goes down. Warning lights continue alternating.
        *   **Stage 3: The Wait:** Animation complete. Warning lights turn OFF. Transitions to `EndOfTurnPhase`.

### EndOfTurnPhase
*   **Why:** Implements a single unified state to wait for turn dismissal across all in-game scoring outcomes.
*   **Defined in:** `src/farkle/include/phases/EndOfTurnPhase.h` & `src/farkle/src/phases/EndOfTurnPhase.cpp`
*   **Implementation Details:**
    1.  **Wait for Dismissal:** The `update()` method waits for any button press (`input.action != NONE`).
    2.  **Finalize Turn:** Once a button is pressed:
        a. Check if `!state.finalRoundTriggered` and if the current player's score is now `>= state.targetScore`. If so, set `state.finalRoundTriggered = true;`.
        b. Call the shared helper `this->endTurn(state);` to advance the `currentPlayerIndex`.
        c. `return game.getPhase<WaitingPhase>();`.

*   **Refactoring Note:** To support the unique display requirements (flashing score, alternating lights, and conditional at-risk display), `InGamePhase::display()` is refactored into smaller virtual hooks:
    *   `updateWarningLights()`: Collects the `farkle_count` for all players and the `currentPlayerIndex`. It passes this data to the `FarkleWarningLights` component to update the entire 8-LED Status Strip (current player flashing, others dim/solid).
    *   `updateScoreDisplays()`: Decomposed into sub-hooks for the three segments: `updateAtRiskScoreDisplay()`, `updateCurrentPlayerScoreDisplay()`, and `updateCompetitionScoreDisplay()`.

### Visual Feedback
-   **Unified Color Identity:** The `TextDisplayV2` renders the active player's name and critical turn information in the `Player.color` stored in the `GameState`. This matches the player's row on the `LedProgressGrid`, creating a cohesive visual link between the scoreboard and the high-resolution UI.
-   **Turn Indicator (FarkleWarningLights):**
    -   **WaitingPhase:** The current player's LED blinks (White/Yellow/Red) to indicate it is their turn to act.
    -   **Banking/Farkling Phases:** The current player's LED becomes solid (like other players) during animations, reducing visual noise.
    -   **PenaltyFarklingPhase:** Triggers a special alternating animation during the penalty sequence.
-   **LED Progress Grid Animations:**
    -   **WaitingPhase:** Shows the banked score and blinks the potential score (`atRiskScore`).
    -   **BankingPhase:** Shows only the banked score growing smoothly. No blinking score.
    -   **FarklingPhase:** Shows the potential score (banked + `atRiskScore`) shrinking smoothly back to the banked score. No blinking score.
    -   **PenaltyFarklingPhase:** Shows only the banked score shrinking smoothly. No blinking score.

### Score Display Behavior
- **Default (InGamePhase):** If `atRiskScore` is 0, the display is cleared.
- **WaitingPhase:** Overrides `updateAtRiskScoreDisplay` to show 0 even when `atRiskScore` is 0.
- **Rationale:** This provides clear visual feedback of when a turn has truly ended and the next turn is ready to begin.
- **Final Round Feedback:** In all `InGamePhase` subclasses, the `COMPETITION_SCORE` display (showing the leading score) will blink if `state.finalRoundTriggered` is true. This signals to the current player that they are in the "bell lap" and must catch the leader.
