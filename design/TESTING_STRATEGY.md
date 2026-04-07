> **Scope:** Defines the complete strategy for testing game logic, component logic, and performance using a "double mocking" approach in a native environment.
> **Status:** **LIVE DOCUMENT** - This file represents the current source of truth. If code changes, this document MUST be updated.

# Testing Strategy


## 1. Overview
Testing embedded software is challenging because the code usually depends on hardware libraries (like `Arduino.h`, `FastLED`, or `Wire.h`) that do not exist on a standard computer. This often leads developers to rely on manual "upload and verify" loops, which are slow and brittle.

This document outlines a **Native Testing Strategy** using the **Unity** framework within PlatformIO. By decoupling our Game Logic from the physical hardware, we can compile and run the entire Game State Machine on a Linux development machine. This allows for:
*   **Instant Feedback:** Tests run in milliseconds, not minutes.
*   **Logic Verification:** We can mathematically prove that scoring, banking, and state transitions work correctly.
*   **Edge Case Simulation:** We can easily simulate rare events (like 3 consecutive Farkles) that are hard to trigger manually.

## 2. Testing Architecture

We will utilize PlatformIO's multi-environment capability to create two distinct build contexts.

### 2.1 The Environments
1.  **`env:nucleo_f401re` (Target)**
    *   **Purpose:** The real firmware running on the hardware.
    *   **Includes:** Real Game Logic + Real Hardware Drivers (`lib/components`).
    *   **Libraries:** `Arduino`, `FastLED`, `U8g2`.

2.  **`env:native` (Host)**
    *   **Purpose:** Unit tests running on your Linux machine.
    *   **Includes:** Real Game Logic + **Fake** Hardware Drivers (`test/mocks`).
    *   **Libraries:** *None* (Standard C++ only).

### 2.2 The "Fake" Driver Strategy (Link-Time Substitution)
To make the Game Logic compile on Linux, we must provide implementations of the hardware classes (`ScoreDisplay`, `ControlPad`, etc.) that mimic the real interfaces but remove the hardware dependencies.

*   **The Real `ScoreDisplay`:** Includes `<FastLED.h>`, sends data to shift registers/LED strips.
*   **The Fake `ScoreDisplay`:** Includes standard `<vector>` or `<string>`, stores the value in a public variable (e.g., `int capturedValue;`).

**Mechanism:**
In `platformio.ini`, we will configure the `native` environment to:
1.  **Ignore** the real component libraries (`lib_ignore`).
2.  **Include** the mock libraries (`test/mocks`).

This effectively "tricks" the `Game` class. When `Game.cpp` calls `scoreDisplay.show(500)`, it thinks it's talking to hardware, but it's actually talking to our Fake, which simply records `lastShownScore = 500`.

## 3. Directory Structure

The test directory is structured to separate mock implementations from the actual tests, and to categorize tests by their scope.

```text
test/
├── README
├── mocks/
│   ├── include/
│   │   ├── Arduino.h
│   │   ├── FastLED.h
│   │   ├── EEPROM.h
│   │   ├── ...
│   └── src/
│       ├── Arduino.cpp
│       └── ...
└── test_game_logic/
    ├── test_main.cpp
    ├── test_utils.cpp
    ├── test_utils.h
    ├── small_tests/
    │   ├── test_BankingPhase.cpp
    │   ├── test_FarklingPhase.cpp
    │   └── ...
    ├── medium_tests/
    │   └── test_turn_lifecycle.cpp
    └── large_tests/
        └── test_full_game.cpp
```

## 4. Test Pyramid Strategy

We will structure our tests into three tiers based on scope and complexity. This ensures we cover individual logic, state handoffs, and full game flows.

### 4.1 SMALL Tests (Unit Tests)
**Focus:** Isolation. Verification of individual `GamePhase` classes.
**Location:** `test/test_game_logic/small_tests/`

*   **`test_WaitingPhase.cpp`**
    *   **`test_WaitingPhase_ScoreAccumulation`:** Verifies that the `atRiskScore` correctly accumulates when score buttons are pressed.
    *   **`test_WaitingPhase_LeaderboardScrolling`:** Verifies leaderboard scrolling behavior.
    *   **`test_WaitingPhase_FinalRoundBlinking`:** Verifies that the Competition Score display blinks when `finalRoundTriggered` is true.
    *   **`test_WaitingPhase_ScoreCorrection`:** Verifies that the `atRiskScore` is cleared when the `CLEAR` button is pressed.
    *   **`test_WaitingPhase_TransitionToBanking`:** Verifies that the game transitions to the `BankingPhase` when the `BANK` button is pressed.
    *   **`test_WaitingPhase_TransitionToFarkling`:** Verifies that the game transitions to the `FarklingPhase` when the `FARKLE` button is pressed.
    *   **`test_WaitingPhase_TransitionToPenaltyFarkling`:** Verifies that PenaltyFarklingPhase IS triggered if the player has 3 consecutive farkles.
    *   **`test_WaitingPhase_GridAnimationScores`:** Verifies that the LedProgressGrid receives the correct scores and YES blinking score during the WaitingPhase.

*   **`test_BankingPhase.cpp`**
    *   **`test_BankingPhase_AnimationMath`:** Verifies that the score animation correctly moves points from `atRiskScore` to the player's score.
    *   **`test_BankingPhase_FinalRoundBlinking`:** Verifies that the Competition Score display blinks when `finalRoundTriggered` is true.
    *   **`test_BankingPhase_ZeroFloorSafety`:** Verifies that `atRiskScore` does not go negative when the points to be moved in one loop are greater than the remaining `atRiskScore`.
    *   **`test_BankingPhase_InputSpamming`:** Verifies that button presses are ignored while the banking animation is in progress.
    *   **`test_BankingPhase_ManualAdvance`:** Verifies that a button press transitions to the `WaitingPhase` after the banking animation is complete.
    *   **`test_BankingPhase_FarkleResetOnEnter`:** Verifies that the player's consecutive farkle count is reset to 0 immediately upon entering the `BankingPhase`.
    *   **`test_BankingPhase_LightsOffDuringAnimation`:** Verifies that the `FarkleWarningLights` are off during the banking animation as a result of the farkle count reset.
    *   **`test_BankingPhase_GridAnimationScores`:** Verifies that the LedProgressGrid receives the correct scores and NO blinking score during the BankingPhase.

*   **`test_FarklingPhase.cpp`**
    *   **`test_FarklingPhase_AnimationMath`:** Verifies that the `atRiskScore` drains to 0 but does NOT add to player score.
    *   **`test_FarklingPhase_FinalRoundBlinking`:** Verifies that the Competition Score display blinks when `finalRoundTriggered` is true.
    *   **`test_FarklingPhase_ZeroFloorSafety`:** Verifies that `atRiskScore` does not go negative.
    *   **`test_FarklingPhase_InputSpamming`:** Verifies that button presses are ignored while the farkling animation is in progress.
    *   **`test_FarklingPhase_ManualAdvance`:** Verifies that a button press transitions to the `WaitingPhase` after the animation is complete.
    *   **`test_FarklingPhase_NoHarmNoFoul_NoIncrement`:** Verifies that if a player has 0 banked points, their `farkle_count` does **not** increment upon farkling.
    *   **`test_FarklingPhase_IncrementWithPoints`:** Verifies that farkle_count DOES increment if the player has points.
    *   **`test_FarklingPhase_GridAnimationScores`:** Verifies that the LedProgressGrid receives the potential score as the base score, and NO blinking score during the FarklingPhase.

*   **`test_PenaltyFarklingPhase.cpp`**
    *   **`test_PenaltyFarklingPhase_AnimationMath`:** Verifies that the score animation correctly moves points from atRiskScore (negative) to 0 and subtracts from the player's score, after the 5-second pause.
    *   **`test_PenaltyFarklingPhase_BlinkParameter`:** Verifies that the blink parameter is correctly passed to the ScoreDisplay during THE_PAIN.
    *   **`test_PenaltyFarklingPhase_ZeroCeilingSafety`:** Verifies that `atRiskScore` does not go above 0.
    *   **`test_PenaltyFarklingPhase_InputSpamming`:** Verifies that button presses are ignored during both the "Pain" and "Drain" stages.
    *   **`test_PenaltyFarklingPhase_ManualAdvance`:** Verifies that a button press transitions to the `WaitingPhase` after the animation is complete.
    *   **`test_PenaltyFarklingPhase_FinalRoundBlinking`:** Verifies that the Competition Score display blinks when `finalRoundTriggered` is true.
    *   **`test_PenaltyFarklingPhase_GridAnimationScores`:** Verifies that the LedProgressGrid receives the correct scores and NO blinking score during the PenaltyFarklingPhase.

*   **`test_EndOfTurnPhase.cpp`**
    *   **`test_EndOfTurnPhase_ManualAdvance`:** Verifies that a button press advances the turn and transitions to `WaitingPhase`.
    *   **`test_EndOfTurnPhase_RotationAdvance`:** Verifies that a rotation advances the turn and transitions to WaitingPhase.
    *   **`test_EndOfTurnPhase_FinalRoundTrigger`:** Verifies that the phase correctly triggers final round if score condition met.
    *   **`test_EndOfTurnPhase_DisplayClearsAtRisk`:** Verifies that the At-Risk display is cleared when the turn ends.
    *   **`test_EndOfTurnPhase_WaitWithoutInput`:** Verifies that the phase properly waits and automatically advances after a 5-second timeout.

*   **`test_TargetScoreSelectionPhase.cpp`**
    *   **`test_TargetScoreSelection_InitialState`**: Verifies that the phase starts with the default target score (10,000).
    *   **`test_TargetScoreSelection_Adjustment`**: Verifies that `rotationDelta` (Encoder) increments and decrements the target score correctly.
    *   **`test_TargetScoreSelection_Clamping`**: Verifies that the target score is clamped between 1,000 and 20,000.
    *   **`test_TargetScoreSelection_Transition`**: Verifies that pressing `BANK`, `FARKLE`, or `SELECT` transitions to `PlayerSelectionPhase`.

*   **`test_PlayerSelectionPhase.cpp`**
    *   **`test_PlayerSelection_InitialState`**: Verifies that the phase starts with the first name in the pool and an empty player list (after transitioning from target selection).
    *   **`test_PlayerSelection_Cycling`**: Verifies that `rotationDelta` (Encoder) navigates the filtered name list correctly, including wrapping behavior.
    *   **`test_PlayerSelection_AddPlayer`**: Verifies that pressing **BANK** (Green) adds the selected name to the `GameState`, assigns a color in the `LedProgressGrid`, and removes the name from the selection list.
    *   **`test_PlayerSelection_Filtering`**: Verifies that multiple added players are all removed from the selection list and index stays valid.
    *   **`test_PlayerSelection_MaxPlayers`**: Verifies that the phase respects the hardware limit by disabling player addition once the `LedProgressGrid` is full (8 players).
    *   **`test_PlayerSelection_TransitionValidation`**: Verifies that pressing **FARKLE** (Red) is ignored if the player list is empty, but successfully transitions to `WaitingPhase` if at least one player exists.
    *   **`test_PlayerSelection_AddLastPlayerWraps`**: Verifies that adding the last name in the filtered pool correctly wraps the selection index back to the first available name (index 0).

*   **`test_multi_press.cpp`** (Test Utilities Verification)
    *   **`test_simulate_button_press_count`**: Verifies that `simulateButtonPress` correctly interprets the optional `count` parameter to trigger multiple button presses in sequence, ensuring the test utility functions as intended.

### 4.2 MEDIUM Tests (Integration Tests)
**Focus:** Handoffs. Verification of state persistence across phase transitions.
**Location:** `test/test_game_logic/medium_tests/`

*   **`test_turn_lifecycle.cpp`**
    *   **`test_TurnLifecycle_FullSetupAndTurn`**: Enhanced to start in `TargetScoreSelectionPhase`, transition through `PlayerSelectionPhase`, add specific players, and verify that those specific players are the ones active during the `WaitingPhase`.
    *   **`test_TurnLifecycle_StandardTurn`**: Verifies that a standard turn correctly banks the score and advances to the next player.
    *   **`test_TurnLifecycle_RoundRobin`**: Verifies that the game correctly cycles through the dynamic list of players created during setup.
    *   **`test_TurnLifecycle_ClearButton`**: Verifies that the clear button resets the `atRiskScore` to 0.

*   **`test_tie_breaking.cpp`**
    *   **`test_TieBreaking_Case1`**: Verifies that if multiple players reach the same score (exactly at target), the first one who reached it in the rotation wins.
    *   **`test_TieBreaking_Case2`**: Verifies that if multiple players reach the same score (above target), the first one who reached it in the rotation wins.
    *   **`test_TieBreaking_Case3`**: Edge case verifying that "first" is calculated relative to the player who triggered the final round, even if the trigger happens late in the roster.

*   **`test_conditional_at_risk_display.cpp`**
    *   **`test_DisplayLogic_PlayerSelection_DisplaysOff`**: Verifies that during the selection phase, `ScoreDisplay` segments and `FarkleWarningLights` are explicitly cleared (except `COMPETITION_SCORE` which shows the target score).
    *   **`test_DisplayLogic_WaitingPhase_ShowsZero`**: Verifies that in `WaitingPhase`, an `atRiskScore` of 0 is displayed as "0" on the `ScoreDisplay`.
    *   **`test_DisplayLogic_BankingPhase_ClearsZero`:** Verifies that in `BankingPhase`, an `atRiskScore` of 0 results in the display being cleared.
    *   **`test_DisplayLogic_FarklingPhase_ClearsZero`:** Verifies that in `FarklingPhase`, an `atRiskScore` of 0 results in the display being cleared.
    *   **`test_DisplayLogic_PenaltyFarklingPhase_ClearsOnlyAtZero`:** Verifies that in `PenaltyFarklingPhase`, the at-risk display remains visible while the score is negative (during "the pain" and "the drain" stages) and only turns off once the animation completes at exactly 0.
    *   **`test_DisplayLogic_InGamePhase_PassesAllFarkleCounts`:** Verifies that the game logic correctly passes all players' farkle counts (not just the current player's) to the `FarkleWarningLights` component, enabling system-wide status display.
    *   **`test_DisplayLogic_BankingPhase_NoBlinking`:** Verifies that during BankingPhase, blinkingPlayerIndex should be -1.
    *   **`test_DisplayLogic_ScoreToggle`:** Verifies that toggling the score display mode between PENDING and BANKED correctly updates the current player's score display.

### 4.3 LARGE Tests (System / End-to-End Tests)
**Focus:** The Macro Game Loop. Win conditions and game completion.
**Location:** `test/test_game_logic/large_tests/`

*   **`test_full_game.cpp`**
    *   **`test_FullGame_StandardGame`**: Replaces hardcoded initialization. The test now simulates the full user journey: Selecting 2-4 players -> Playing until target score -> Winner celebration -> Reset.
    *   **`test_FullGame_TripleFarkle`:** Verifies the triple farkle penalty and reset behavior.
    *   **`test_FullGame_TripleFarkle_ScoreLessThanPenalty`:** Verifies that player score does not go negative when triple farkled.
    *   **`test_FullGame_AutoAdvanceTurn`:** Verifies that a full game can be completed solely via the 5-second automatic timeout for turn advancement.
    *   **`test_FullGame_FinalRoundBlinking`**: Verifies that the Competition Score display begins blinking as soon as the final round is triggered and remains blinking until the game ends.
    *   **`test_FullGame_ScoreToggle`**: Verifies a full game loop incorporating the Total Score Toggle functionality, ensuring the display reads differently based on the switch state.


## 5. Implementation Steps

1.  **Create `platformio.ini` Native Env:** Configure build flags and library ignores.
2.  **Scaffold Mocks:** Create `test/mocks/include` and copy the public function signatures from `lib/components`. Remove all private hardware variables and includes.
3.  **Implement Fake Logic:** Update `FakeScoreDisplay.cpp` to store values in public variables for inspection.
4.  **Write Tests:** Create `test/test_game_logic/test_transitions.cpp` using the Unity `TEST_ASSERT` macros.
5.  **Run:** Execute `pio test -e native`.

## 6. Component Performance Testing

To verify the performance and correctness of individual hardware components (like `ScoreDisplay`) without flashing hardware, we use a specialized test environment `env:component_tests`.

### 6.1 Strategy: "Double Mocking"
We employ a two-layer mocking strategy to isolate different parts of the system:
1.  **Game Logic Tests (`env:native`):** We mock the *Component* (e.g., `FakeScoreDisplay`). This assumes the component works and tests the game rules.
2.  **Component Tests (`env:component_tests`):** We use the **REAL** component code (`ScoreDisplay.cpp`, `TextDisplayV2.cpp`) but mock the **External Hardware Library** (e.g., `SPI.h`, `Adafruit_ST7789.h`).

### 6.2 Why this is needed
*   **Hardware-Independent Correctness:** Verifies logic like "splitting a 5-digit number into characters" or "padding with spaces" works correctly before it touches a real chip.
*   **Performance Benchmarking:** Tests can use `std::chrono` to measure execution time (e.g., "10,000 writes in 500us"), helping us catch regressions in critical loops.

### 6.3 Running Component Tests
To run these tests, use the PlatformIO CLI:
```bash
pio test -e component_tests
```

### 6.4 Directory Structure
*   `test/test_component_*/`: Contains the test suites for components (e.g., `test_component_score_display/`, `test_component_led_progress_grid/`).
    *   **Note on Subdirectories**: Individual components have their own test subdirectories to avoid linker conflicts. The Unity framework's `setUp`, `tearDown`, and `main` functions would otherwise cause multiple-definition errors when compiling all component tests into a single test runner.
*   `test/mocks/libs/`: Contains mocks for external libraries (e.g., `SPI.h`, `Adafruit_NeoPixel.h`, `Adafruit_ST7789.h`) used by components.

### 6.5 Example Test Cases: `ScoreDisplay`
*   **`test_ScoreDisplay_Correctness_Overflow`**: Verifies that numbers greater than 99,999 are capped and displayed as "99999".
*   **`test_ScoreDisplay_Blinking_Intensity`**: Verifies that when `blink` is enabled, calling `print_number` (which should be called every frame) results in the intensity toggling between `SCORE_BLINK_LOW` (2) and `SCORE_BLINK_HIGH` (12) as time advances.
*   **`test_ScoreDisplay_HardwareInteractionOptimization`**: Verifies that the internal state caching prevents redundant hardware calls when the requested state (number, blink mode, intensity) has not changed.
*   **`test_ScoreDisplay_Performance`**: Benchmarks the execution time of repeated display updates to ensure high-speed SPI transactions perform adequately in critical loops.
*   **`test_ScoreDisplay_Security_InvalidType`**: Verifies that passing an invalid `DisplayType` enum value does not crash the system and is gracefully rejected.
*   **`test_ScoreDisplay_Security_NegativeOverflow`**: Verifies that deeply negative numbers (e.g., -12345, which would require 6 digits including the sign) are clamped to `-9999` to prevent out-of-bounds memory writes when formatting the display segments.

### 6.6 Example Test Cases: `TextDisplayV2`
*   **`test_begin`**: Verifies that the display initialization sequence correctly configures the Adafruit ST7789 library (init, setRotation, fillScreen).
*   **`test_print`**: Verifies that `print()` passes the correct text content and mapped RGB565 color to the underlying Adafruit display graphics API.
*   **`test_print_caching`**: Verifies that repeated calls to `print()` with the exact same text and color do not trigger redundant hardware rendering.
*   **`test_printSelectionScreen`**: Verifies that the selection screen correctly renders both the title text and the item text, along with generating the geometric up/down arrow indicators.
*   **`test_mode_switching`**: Verifies that transitioning between different display modes (e.g., MESSAGE to SELECTION) correctly invalidates the cache and forces a full screen redraw even if text overlaps.
*   **`test_selection_screen_caching`**: Verifies that state caching works correctly for the interactive selection screen to optimize layout updates.
*   **`test_colorHSVtoRGB565`**: Verifies that the custom, lightweight hue-to-RGB565 math correctly converts extreme inputs, mapping Hue 0 correctly to Red (0xF800).

### 6.7 Example Test Cases: `ControlPad`
*   **`test_add_valid_button`**: Verifies that adding a button with a valid pin correctly configures the pin mode to `INPUT_PULLUP`.
*   **`test_add_invalid_pin_negative`**: Verifies that adding a button with a negative pin index is ignored and does not corrupt memory or configure hardware.
*   **`test_read_valid_button`**: Verifies that reading the control pad correctly detects a button press (LOW state) and returns the associated action.

### 6.8 Example Test Cases: `LedProgressGrid`
*   **`test_LedProgressGrid_MaxScore_Exact`**: Verifies that `_maxScore` scales exactly to the highest score (if above target) rather than jumping by fixed increments.
*   **`test_LedProgressGrid_MaxScore_IncludesAtRisk`**: Verifies that the dynamic grid maximum correctly incorporates the current player's at-risk points.
*   **`test_LedProgressGrid_MaxScore_Shrinking`**: Verifies that the grid bounds can shrink back to the target score if a leading player farkles or a turn ends without banking.

### 6.9 Example Test Cases: `FarkleWarningLights`
*   **`test_Update_SetsCorrectColorsAndBrightness`**: Verifies that the component correctly sets NeoPixel colors and brightness based on player status (Active/Idle) and farkle count (0: White/Off, 1: Yellow, 2+: Red).
*   **`test_MultiLedMapping`**: Verifies that the component uses the shared `PlayerLayout` to map a single player to multiple LEDs when fewer than 8 players are present.
*   **`test_BlinkLogic`**: Verifies that the active player's LEDs blink (toggle On/Off) based on the `isBlinking` parameter, while idle players remain solid.
*   **`test_Alternate_SmoothTransition`**: Verifies that the warning light smoothly transitions between Red and Yellow over a 1000ms cycle during the catastrophic penalty phase.


## 7. Hybrid Input & Parallel Architecture (v2)

With the move to the Hybrid Input Model, testing must ensure that digital actions, analog ladder values, and encoder rotations are resolved correctly.

### 7.1 Priority Matrix Testing
Mocks must simulate simultaneous inputs to verify the following priority rules:
1.  **Digital Dominance**: If `BANK`, `FARKLE`, or `SELECT` (Digital) is pressed, any simultaneous `PLUS_XXX` (Analog) or `Rotation` (Encoder) signals must be suppressed or ignored in the `GameInput` result for that frame.
2.  **Exclusive Discrete Action**: The `read()` method must return only ONE `ButtonAction` per frame.

### 7.2 Temporal Stability (Analog Ladder)
To filter noise from NeoPixel power draw, the Analog Ladder (A2) requires temporal validation:
*   **Stability Window**: An ADC value must remain within a specific zone (e.g., the `PLUS_50` zone) for **exactly 50ms** before a `ButtonAction` is emitted.
*   **Noise Rejection**: Mocks must verify that a 40ms pulse in an ADC zone is ignored.

### 7.3 Encoder & Buffer Validation
*   **Atomic Consumption**: Verify that `read()` returns the *full* `rotationDelta` accumulated since the last call and resets the internal atomic buffer to zero.
*   **Directionality**: Ensure `rotationDelta` correctly reflects positive (clockwise) and negative (counter-clockwise) ticks.

### 7.4 No-Repeat & Undefined States
*   **Single Trigger**: Holding an analog button (e.g., `PLUS_50`) must result in exactly one action. A return to the "Neutral" ADC zone is required before another action can be triggered.
*   **Dead Zones**: Verify that ADC values between defined zones return `ButtonAction::NONE`.
