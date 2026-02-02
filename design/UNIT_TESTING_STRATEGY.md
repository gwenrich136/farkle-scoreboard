# Unit Testing Strategy

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

We will introduce a `test` directory structure to support this:

```text
test/
├── README.md
├── mocks/                  <-- The "Fake" Hardware Layer
│   ├── include/
│   │   ├── Arduino.h       <-- A minimal fake Arduino header (defining millis(), delay(), etc.)
│   │   ├── ScoreDisplay.h  <-- Same public API as real ScoreDisplay, no FastLED
│   │   └── ...
│   └── src/
│       ├── Arduino.cpp
│       ├── ScoreDisplay.cpp
│       └── ...
├── test_game_logic/        <-- The actual Unit Tests
│   ├── test_main.cpp       <-- Unity test runner
│   ├── test_transitions.cpp
│   └── test_scoring.cpp
```

## 4. Test Pyramid Strategy

We will structure our tests into three tiers based on scope and complexity. This ensures we cover individual logic, state handoffs, and full game flows.

### 4.1 SMALL Tests (Unit Tests)
**Focus:** Isolation. Verification of individual `GamePhase` classes.
**Location:** `test/test_small/`

*   **`test_WaitingPhase.cpp`**
    *   **Scoring Math:** 
        *   Simulate `UP_1000`. Assert `atRiskScore == 1000`.
        *   Simulate `RIGHT_500`. Assert `atRiskScore == 1500`.
    *   **Correction:** 
        *   Set `atRiskScore = 500`. Simulate `CLEAR`. Assert `atRiskScore == 0`.
    *   **Transitions:**
        *   Simulate `BANK`. Assert `update()` returns `BankingPhase*`.
        *   Simulate `FARKLE`. Assert `update()` returns `FarklingPhase*`.

*   **`test_BankingPhase.cpp`**
    *   **Animation Math:** 
        *   Set `atRisk = 500`, `banked = 0`.
        *   Call `update(dt=100)`. Assert `atRisk < 500` and `banked > 0` and `atRisk + banked == 500`.
    *   **Zero-Floor Safety:**
        *   Set `atRisk = 50`. Call `update(dt=huge_value)`. 
        *   Assert `atRisk == 0` (not negative) and `banked` increased by exactly 50.
    *   **Input Spamming:** 
        *   While `atRisk > 0`, simulate `BANK`, `CLEAR`, `UP`. 
        *   Assert phase does *not* transition and score logic is unaffected.
    *   **Manual Advance:** 
        *   Set `atRisk = 0`. Call `update()`. Assert returns `this` (no transition).
        *   Simulate `BANK`. Assert returns `WaitingPhase*`.

*   **`test_FarklingPhase.cpp`**
    *   **Standard Farkle:** 
        *   Set `atRisk = 500`. Call `update()` until done.
        *   Assert `atRisk == 0` and `banked` is unchanged.
        *   Assert `state.players[state.currentPlayerIndex].farkle_count` increased by 1.
    *   **Zero-Floor Safety:**
        *   Set `atRisk = 50`. Call `update(dt=huge_value)`.
        *   Assert `atRisk == 0` (not negative).
    *   **Triple Farkle (Penalty):** 
        *   Set `state.players[0].farkle_count = 2`.
        *   Trigger transition.
        *   Assert `state.players[0].score` decreases by 1000 (or `atRisk` logic handles it).
    *   **Exit Transition:** Verify button press triggers transition to `WaitingPhase`.

### 4.2 MEDIUM Tests (Integration Tests)
**Focus:** Handoffs. Verification of state persistence across phase transitions.
**Location:** `test/test_medium/`

*   **`test_turn_lifecycle.cpp`**
    *   **Scenario: Standard Turn**
        1.  Start as P1. Score 500. Trigger `BankingPhase`.
        2.  Run loop until animation completes.
        3.  Trigger manual dismiss.
        4.  **Assert:** `state.currentPlayerIndex == 1` (P2). P1 score is 500. `atRiskScore == 0`.
    *   **Scenario: The Triple Farkle**
        1.  Set P1 `farkle_count = 2`, `score = 2000`.
        2.  Trigger `FarklingPhase`. Run animation & dismiss.
        3.  **Assert:** P1 score is 1000 (-1000 penalty). `currentPlayerIndex == 1`.
    *   **Scenario: Round Robin**
        1.  Complete turns for P1, P2, P3.
        2.  Complete turn for P4.
        3.  **Assert:** `currentPlayerIndex == 0` (Back to P1).

### 4.3 LARGE Tests (System / End-to-End Tests)
**Focus:** The Macro Game Loop. Win conditions and game completion.
**Location:** `test/test_large/`

*   **`test_full_game.cpp`**
    *   **Scenario: The Standard Game**
        1.  P1 scores `>= state.targetScore`. 
        2.  **Assert:** `finalRoundTriggered == true`.
        3.  Play turns for P2, P3, P4.
        4.  Attempt to play next turn.
        5.  **Assert:** Current phase is `PostGamePhase`.
    *   **Scenario: The Overtake**
        1.  P1 scores `state.targetScore`. `finalRoundTriggered = true`.
        2.  P2 scores `state.targetScore + 1000`.
        3.  **Assert:** `finalRoundTriggered` remains `true`.
        4.  Play P3, P4.
        5.  **Assert:** Game ends (does not extend for P2).


## 5. Implementation Steps

1.  **Create `platformio.ini` Native Env:** Configure build flags and library ignores.
2.  **Scaffold Mocks:** Create `test/mocks/include` and copy the public function signatures from `lib/components`. Remove all private hardware variables and includes.
3.  **Implement Fake Logic:** Update `FakeScoreDisplay.cpp` to store values in public variables for inspection.
4.  **Write Tests:** Create `test/test_game_logic/test_transitions.cpp` using the Unity `TEST_ASSERT` macros.
5.  **Run:** Execute `pio test -e native`.
