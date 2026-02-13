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
    *   **`test_WaitingPhase_ScoreCorrection`:** Verifies that the `atRiskScore` is cleared when the `CLEAR` button is pressed.
    *   **`test_WaitingPhase_TransitionToBanking`:** Verifies that the game transitions to the `BankingPhase` when the `BANK` button is pressed.
    *   **`test_WaitingPhase_TransitionToFarkling`:** Verifies that the game transitions to the `FarklingPhase` when the `FARKLE` button is pressed.

*   **`test_BankingPhase.cpp`**
    *   **`test_BankingPhase_AnimationMath`:** Verifies that the score animation correctly moves points from `atRiskScore` to the player's score.
    *   **`test_BankingPhase_ZeroFloorSafety`:** Verifies that `atRiskScore` does not go negative when the points to be moved in one loop are greater than the remaining `atRiskScore`.
    *   **`test_BankingPhase_InputSpamming`:** Verifies that button presses are ignored while the banking animation is in progress.
    *   **`test_BankingPhase_ManualAdvance`:** Verifies that a button press transitions to the `WaitingPhase` after the banking animation is complete.
    *   **`test_BankingPhase_FarkleResetOnEnter`:** Verifies that the player's consecutive farkle count is reset to 0 immediately upon entering the `BankingPhase`.
    *   **`test_BankingPhase_LightsOffDuringAnimation`:** Verifies that the `FarkleWarningLights` are off during the banking animation as a result of the farkle count reset.

*   **`test_FarklingPhase.cpp`**
    *   **`test_FarklingPhase_AnimationMath`:** Verifies that the `atRiskScore` drains to 0 but does NOT add to player score.
    *   **`test_FarklingPhase_ZeroFloorSafety`:** Verifies that `atRiskScore` does not go negative.
    *   **`test_FarklingPhase_InputSpamming`:** Verifies that button presses are ignored while the farkling animation is in progress.
    *   **`test_FarklingPhase_ManualAdvance`:** Verifies that a button press transitions to the `WaitingPhase` after the animation is complete.
    *   **`test_FarklingPhase_NoHarmNoFoul`:** Verifies that if a player has 0 banked points, their `farkle_count` does **not** increment upon farkling.

*   **`test_PenaltyFarklingPhase.cpp`**
    *   **`test_PenaltyFarklingPhase_Stage1_ThePain`:** Verifies that for the first 3 seconds, the `atRiskScore` flashes/blinks, the `FarkleWarningLights` alternate, and **no points are moved**.
    *   **`test_PenaltyFarklingPhase_Stage2_TheDrain`:** Verifies that after the initial delay, points begin to move (inverse banking) while lights continue to alternate.
    *   **`test_PenaltyFarklingPhase_Stage3_TheWait`:** Verifies that once the score is 0, the animation stops, lights turn OFF, and the game waits for a manual advance button.
    *   **`test_PenaltyFarklingPhase_ZeroCeilingSafety`:** Verifies that `atRiskScore` does not go above 0.
    *   **`test_PenaltyFarklingPhase_InputSpamming`:** Verifies that button presses are ignored during both the "Pain" and "Drain" stages.
    *   **`test_PenaltyFarklingPhase_ManualAdvance`:** Verifies that a button press transitions to the `WaitingPhase` after the animation is complete.

*   **`test_TargetScoreSelectionPhase.cpp`**
    *   **`test_TargetScoreSelection_InitialState`**: Verifies that the phase starts with the default target score (10,000).
    *   **`test_TargetScoreSelection_Adjustment`**: Verifies that `UP_1000` and `DOWN_50` increment and decrement the target score correctly.
    *   **`test_TargetScoreSelection_Clamping`**: Verifies that the target score is clamped between 1,000 and 20,000.
    *   **`test_TargetScoreSelection_Transition`**: Verifies that pressing `BANK` or `FARKLE` transitions to `PlayerSelectionPhase`.

*   **`test_PlayerSelectionPhase.cpp`**
    *   **`test_PlayerSelection_InitialState`**: Verifies that the phase starts with the first name in the pool and an empty player list (after transitioning from target selection).
    *   **`test_PlayerSelection_Cycling`**: Verifies that `UP_1000` and `DOWN_50` navigate the filtered name list correctly, including wrapping behavior.
    *   **`test_PlayerSelection_AddPlayer`**: Verifies that pressing **BANK** (Green) adds the selected name to the `GameState`, assigns a color in the `LedProgressGrid`, and removes the name from the selection list.
    *   **`test_PlayerSelection_MaxPlayers`**: Verifies that the phase respects the hardware limit by disabling player addition once the `LedProgressGrid` is full (8 players).
    *   **`test_PlayerSelection_TransitionValidation`**: Verifies that pressing **FARKLE** (Red) is ignored if the player list is empty, but successfully transitions to `WaitingPhase` if at least one player exists.
    *   **`test_PlayerSelection_AddLastPlayerWraps`**: Verifies that adding the last name in the filtered pool correctly wraps the selection index back to the first available name (index 0).

### 4.2 MEDIUM Tests (Integration Tests)
**Focus:** Handoffs. Verification of state persistence across phase transitions.
**Location:** `test/test_game_logic/medium_tests/`

*   **`test_turn_lifecycle.cpp`**
    *   **`test_TurnLifecycle_FullSetupAndTurn`**: Enhanced to start in `TargetScoreSelectionPhase`, transition through `PlayerSelectionPhase`, add specific players, and verify that those specific players are the ones active during the `WaitingPhase`.
    *   **`test_TurnLifecycle_RoundRobin`**: Verifies that the game correctly cycles through the dynamic list of players created during setup.
    *   **`test_TurnLifecycle_ClearButton`**: Verifies that the clear button resets the `atRiskScore` to 0.

*   **`test_conditional_at_risk_display.cpp`**
    *   **`test_DisplayLogic_PlayerSelection_DisplaysOff`**: Verifies that during the selection phase, `ScoreDisplay` segments and `FarkleWarningLights` are explicitly cleared (except `COMPETITION_SCORE` which shows the target score).
    *   **`test_DisplayLogic_WaitingPhase_ShowsZero`**: Verifies that in `WaitingPhase`, an `atRiskScore` of 0 is displayed as "0" on the `ScoreDisplay`.
    *   **`test_DisplayLogic_BankingPhase_ClearsZero`:** Verifies that in `BankingPhase`, an `atRiskScore` of 0 results in the display being cleared.
    *   **`test_DisplayLogic_FarklingPhase_ClearsZero`:** Verifies that in `FarklingPhase`, an `atRiskScore` of 0 results in the display being cleared.
    *   **`test_DisplayLogic_PenaltyFarklingPhase_ClearsOnlyAtZero`:** Verifies that in `PenaltyFarklingPhase`, the at-risk display remains visible while the score is negative (during "the pain" and "the drain" stages) and only turns off once the animation completes at exactly 0.
    *   **`test_CompetitionScore_NoBlink_NormalRound`**: Verifies that the leading score display does NOT blink during a normal game round.
    *   **`test_CompetitionScore_Blinks_FinalRound`**: Verifies that the leading score display blinks during the final round when `finalRoundTriggered` is true.

### 4.3 LARGE Tests (System / End-to-End Tests)
**Focus:** The Macro Game Loop. Win conditions and game completion.
**Location:** `test/test_game_logic/large_tests/`

*   **`test_full_game.cpp`**
    *   **`test_FullGame_StandardGame`**: Replaces hardcoded initialization. The test now simulates the full user journey: Selecting 2-4 players -> Playing until target score -> Winner celebration -> Reset.
    *   **`test_FullGame_TripleFarkle`:** Verifies the triple farkle penalty and reset behavior.


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
2.  **Component Tests (`env:component_tests`):** We use the **REAL** component code (`ScoreDisplay.cpp`) but mock the **External Hardware Library** (e.g., `LedControl`).

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
*   `test/mocks/libs/`: Contains mocks for external libraries (e.g., `LedControl.h`, `Adafruit_NeoPixel.h`) used by components.

### 6.5 Example Test Cases: `ScoreDisplay`
*   **`test_ScoreDisplay_Correctness_Overflow`**: Verifies that numbers greater than 99,999 are capped and displayed as "99999".
*   **`test_ScoreDisplay_Blinking_Intensity`**: Verifies that when `blink` is enabled, calling `print_number` (which should be called every frame) results in the intensity toggling between `SCORE_BLINK_LOW` (2) and `SCORE_BLINK_HIGH` (12) as time advances.

### 6.6 Example Test Cases: `ControlPad`
*   **`test_add_valid_button`**: Verifies that adding a button with a valid pin correctly configures the pin mode to `INPUT_PULLUP`.
*   **`test_add_invalid_pin_negative`**: Verifies that adding a button with a negative pin index is ignored and does not corrupt memory or configure hardware.
*   **`test_read_valid_button`**: Verifies that reading the control pad correctly detects a button press (LOW state) and returns the associated action.
