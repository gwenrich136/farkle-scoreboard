# Testing Strategy

> **Note:** This document is a living document. If you add a new test, please update this document to reflect the changes.

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

*   **`test_FarklingPhase.cpp`**
    *   **`test_FarklingPhase_AnimationMath`:** Verifies that the `atRiskScore` drains to 0 but does NOT add to player score.
    *   **`test_FarklingPhase_ZeroFloorSafety`:** Verifies that `atRiskScore` does not go negative.
    *   **`test_FarklingPhase_InputSpamming`:** Verifies that button presses are ignored while the farkling animation is in progress.
    *   **`test_FarklingPhase_ManualAdvance`:** Verifies that a button press transitions to the `WaitingPhase` after the animation is complete.

*   **`test_PenaltyFarklingPhase.cpp`**
    *   **`test_PenaltyFarklingPhase_AnimationMath`:** Verifies that the score animation correctly moves points from `atRiskScore` (negative) to 0 and subtracts from the player's score.
    *   **`test_PenaltyFarklingPhase_ZeroCeilingSafety`:** Verifies that `atRiskScore` does not go above 0.
    *   **`test_PenaltyFarklingPhase_InputSpamming`:** Verifies that button presses are ignored while the animation is in progress.
    *   **`test_PenaltyFarklingPhase_ManualAdvance`:** Verifies that a button press transitions to the `WaitingPhase` after the animation is complete.

### 4.2 MEDIUM Tests (Integration Tests)
**Focus:** Handoffs. Verification of state persistence across phase transitions.
**Location:** `test/test_game_logic/medium_tests/`

*   **`test_turn_lifecycle.cpp`**
    *   **`test_TurnLifecycle_StandardTurn`:** Verifies that a standard turn correctly banks the score and advances to the next player.
    *   **`test_TurnLifecycle_RoundRobin`:** Verifies that the game correctly cycles through all players.
    *   **`test_TurnLifecycle_ClearButton`:** Verifies that the clear button resets the `atRiskScore` to 0.

### 4.3 LARGE Tests (System / End-to-End Tests)
**Focus:** The Macro Game Loop. Win conditions and game completion.
**Location:** `test/test_game_logic/large_tests/`

*   **`test_full_game.cpp`**
    *   **`test_FullGame_StandardGame`:** Simulates a full game where players take turns scoring until one player reaches the target score, triggering the final round.
    *   **`test_FullGame_TripleFarkle`:** Verifies the triple farkle penalty and reset behavior.


## 5. Implementation Steps

1.  **Create `platformio.ini` Native Env:** Configure build flags and library ignores.
2.  **Scaffold Mocks:** Create `test/mocks/include` and copy the public function signatures from `lib/components`. Remove all private hardware variables and includes.
3.  **Implement Fake Logic:** Update `FakeScoreDisplay.cpp` to store values in public variables for inspection.
4.  **Write Tests:** Create `test/test_game_logic/test_transitions.cpp` using the Unity `TEST_ASSERT` macros.
5.  **Run:** Execute `pio test -e native`.

## 6. Component Performance Testing

To verify the performance and correctness of individual hardware components (like `ScoreDisplay`) without flashing hardware, we use a specialized test environment `env:component_tests`.

### 6.1 Strategy
*   **Real Component Code:** compiled directly from `lib/components`.
*   **Mocked Dependencies:** External hardware libraries (like `LedControl`) are mocked in `test/mocks/libs`.
*   **Performance Benchmarking:** Tests include timing measurements to detect regressions or verify optimizations.

### 6.2 Running Component Tests
To run these tests, use the PlatformIO CLI:
```bash
pio test -e component_tests
```

### 6.3 Directory Structure
*   `test/test_component_tests/`: Contains the test suites for components.
*   `test/mocks/libs/`: Contains mocks for external libraries used by components.
