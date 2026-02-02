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

## 4. Test Scenarios

We will focus our testing efforts on the **Game Logic** and **State Machine**, as these are the most complex and critical parts of the application.

### 4.1 State Transitions (`test_transitions.cpp`)
*   **Waiting -> Banking:** Simulate pressing `BANK`. Verify `Game::currentPhase` changes to `BankingPhase`.
*   **Waiting -> Farkling:** Simulate pressing `FARKLE`. Verify `Game::currentPhase` changes to `FarklingPhase`.
*   **Banking -> Waiting:** Simulate calling `update()` repeatedly until animation finishes + button press. Verify return to `WaitingPhase` and `currentPlayerIndex` increment.

### 4.2 Scoring Logic (`test_scoring.cpp`)
*   **Input Accumulation:** Simulate `UP` (+1000), `RIGHT` (+500). Verify `state.atRiskScore` is 1500.
*   **Correction:** Simulate `CLEAR`. Verify `state.atRiskScore` returns to 0.
*   **Banking Math:** Verify that after `BankingPhase`, `state.players[0].score` increases exactly by the amount `state.atRiskScore` decreased.

### 4.3 Win Conditions (`test_win_condition.cpp`)
*   **Trigger Final Round:** Set Player 1's score to 4950. Bank 100 points. Verify `state.finalRoundTriggered` becomes `true`.
*   **End Game:** Set `finalRoundTriggered = true`. Advance through all players. Verify transition to `PostGamePhase`.

## 5. Implementation Steps

1.  **Create `platformio.ini` Native Env:** Configure build flags and library ignores.
2.  **Scaffold Mocks:** Create `test/mocks/include` and copy the public function signatures from `lib/components`. Remove all private hardware variables and includes.
3.  **Implement Fake Logic:** Update `FakeScoreDisplay.cpp` to store values in public variables for inspection.
4.  **Write Tests:** Create `test/test_game_logic/test_transitions.cpp` using the Unity `TEST_ASSERT` macros.
5.  **Run:** Execute `pio test -e native`.
