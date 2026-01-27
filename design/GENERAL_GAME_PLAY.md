# General Gameplay and User Experience Design for Farkle Scoreboard

This document outlines the desired user experience and general gameplay flow for the Farkle Scoreboard device. It focuses on how the game should behave from the player's perspective, without delving into specific technical implementation details.

---

## 1. Pre-Game Experience

The game begins by guiding players through setup and configuration.

### 1.1 Player Selection
-   **Initial State:** The scoreboard displays a "Player Selection" screen, presenting a list of pre-stored player names and an option to "Add New Player."
-   **Navigation:** Players use `Up/Down` buttons to scroll through the available names. While scrolling, the `LedProgressGrid` visually indicates the "pending player" by blinking the rows that player would occupy.
-   **Selecting an Existing Player:** Pressing the `BANK` (Green) button on a highlighted name adds that player to the game. The player list refreshes, removing the selected player, and the system remains in the "Player Selection" state, ready for the next player.
-   **Adding a New Player:** Selecting the "Add New Player" option (by pressing `BANK`) transitions the device to the "Player Creation" screen.

### 1.2 Player Creation
-   **Purpose:** Allows users to input a new player name character by character.
-   **Input Method:** The screen (via `TextDisplay`) shows the name being built. The active character is highlighted and can be changed by pressing `Up/Down`. Players navigate through the name using `Left/Right` buttons.
-   **Confirmation:** Pressing `BANK` confirms the new name, adds the player to the game, and returns to the "Player Selection" screen.
-   **Aborting:** Pressing `CLEAR` discards the new name and returns to the "Player Selection" screen without adding a player.

### 1.3 Starting the Game
-   **Direct Start:** From the "Player Selection" screen, pressing the `FARKLE` (Red) button immediately starts the game with the currently added players.
-   **Confirmation Flow:** From the "Player Selection" screen, pressing the `CLEAR` button initiates a confirmation step.
    -   The `TextDisplay` asks: "Start {player_count} player game?".
    -   Pressing `FARKLE` or `BANK` starts the game.
    -   Pressing `CLEAR` again cancels and returns to the "Player Selection" screen.

---

## 2. In-Game Experience

### 2.1 Player Turn Flow
-   **Turn Start:** The `TextDisplay` clearly indicates whose turn it is (e.g., "PLAYER 1'S TURN").
-   **Dice Rolling:** Players roll physical dice.
-   **Score Input:**
    -   Players use the directional buttons on the `ControlPad` to add points to their `at-risk` score. These buttons map to common Farkle score increments:
        -   `DOWN`: +50 points
        -   `LEFT`: +100 points
        -   `RIGHT`: +500 points
        -   `UP`: +1000 points
    -   The `atRisk` score is displayed on a `ScoreDisplay` segment and as a blinking section on the `LedProgressGrid`.
    -   **Correction:** Pressing `CLEAR` resets the `atRisk` score to 0 for the current roll, allowing the player to re-enter their score.

### 2.2 Score Animations
-   **Banking Animation:**
    -   When a player presses `BANK`, their `atRisk` score slowly drains to zero while their banked score simultaneously increases by the same amount. Both the `ScoreDisplay` segments and the `LedProgressGrid` animate this proportional change, with the `atRisk` portion shrinking and the banked portion growing.
    -   Input is locked during this animation.
-   **Standard Farkle Animation:**
    -   If a player farkles (but it's not their third consecutive farkle), their `atRisk` score slowly drains to zero.
    -   A witty quip related to farkling is displayed on the `TextDisplay`.
    -   The `FarkleWarningLights` update to reflect the new farkle count.
    -   Input is locked during this animation, followed by a brief pause before the next player's turn begins.

### 2.3 Catastrophic Farkle (Third Consecutive Farkle)
-   **Trigger:** When a player gets their third consecutive farkle in a turn.
-   **Dramatic Animation:**
    -   The `TextDisplay` displays a penalty quip.
    -   The `atRisk` score display immediately begins flashing the actual penalty amount (e.g., "-850" if the banked score is 850 and the penalty would be 1000). This flashing is synchronized with the `FarkleWarningLights` alternating between yellow and red. This state lasts for 3-5 flashes.
    -   After the flashing stops, two animations occur *simultaneously*:
        1.  The `atRisk` display slowly climbs from the negative penalty amount back up to 0.
        2.  The player's banked score (on both `ScoreDisplay` and `LedProgressGrid`) slowly drains by the penalty amount.
    -   Input is locked during this entire sequence. After it completes, there's a brief pause before the next player's turn.

---

## 3. End-Game Experience

### 3.1 The "Last Round" Trigger
-   **Trigger Condition:** The first time any player's banked score exceeds 10,000 points.
-   **Visual & Audio Cue:**
    -   The `ScoreDisplay` segment showing the high score begins to blink continuously.
    -   A "bell sound" plays (future implementation) to signify the final round.
-   **Game Rule:** The game does not end immediately; every other player (including the one who just crossed 10,000) gets one final turn to surpass the highest score.

### 3.2 Game End & Winner Celebration
-   **Game End:** The game concludes after all players have had their final turn. The player with the highest score is declared the winner.
-   **Celebration Sequence:**
    -   The `TextDisplay` prominently displays "{player_name} WINS!".
    -   The entire `LedProgressGrid` performs a dynamic visual celebration (e.g., flashing in the winner's color, or a "fireworks" animation).
    -   The `ScoreDisplay` segment showing the winning score flashes the winning player's final total.

---

## 4. Post-Game Experience

After the winner celebration, the device enters a continuous post-game loop.

### 4.1 Game Replay & Upload (Parallel)
-   **Visual Replay:** The `LedProgressGrid` continuously replays the entire game's score history, showing how each player's score progressed throughout the game.
-   **Data Upload (Background/Interactive):** Simultaneously, the scoreboard attempts to connect via Bluetooth or WiFi to a paired phone.
    -   The `TextDisplay` provides status updates (e.g., "CONNECTING...", "UPLOADING...").
    -   If the upload is successful, the `TextDisplay` indicates this.
    -   If the connection or upload fails, the `TextDisplay` presents a menu of options.

### 4.2 Post-Game Menu
-   **Access:** If the upload fails or the user wishes to perform another action, a menu is presented (using the `displaySelector` UI from pre-game).
-   **Options:** The menu includes, but is not limited to:
    1.  "Retry Upload": Re-attempts the data upload.
    2.  "Play Again": Leads to a "Reset Game?" confirmation.
-   **Navigation:** Users navigate this menu using `Up/Down` and select options with `BANK`.
-   **`LedProgressGrid` Behavior:** The `LedProgressGrid` continues its game replay animation independently while the user interacts with the `TextDisplay` menu.

### 4.3 Reset Game
-   **Confirmation:** Selecting "Play Again" or triggering a reset (e.g., via `CLEAR` from a specific context) leads to a "Reset Game?" confirmation screen.
-   **Action:** Confirming the reset wipes all game history and returns the device to the initial "Player Selection" state.
