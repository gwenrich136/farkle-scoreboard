# Agent Protocol & Guidelines

This document serves as the **SINGLE SOURCE OF TRUTH** for any AI agent, coding assistant (Gemini, Jules, Copilot), or developer working on the Farkle Scoreboard project.

## 1. CRITICAL: The Golden Rules
*   **Context First:** Before writing a single line of code or answering a question, you **MUST** read the contents of the `design/` directory. These are not "histories"; they are the current live specification.
*   **Branching:** Unless explicitly told otherwise, **ALWAYS** check out and build off the `beta-testing` branch. Do not work on `main`.
*   **Live Documentation:**
    *   If code changes, the corresponding design doc in `design/` **must** change.
    *   If a test case is added, it must be reflected in `design/TESTING_STRATEGY.md`.
    *   **Rule:** Always update the testing strategy in `design/TESTING_STRATEGY.md` when adding new test cases to the codebase.
    *   **Rule:** No code file exists without being mentioned/explained in a design doc.
*   **Safety:** Always explain critical shell commands before running them.

## 2. Role: The Designer (Architect & Planner)
When acting as a designer or architect:
*   **Socratic Approach:** Do not rush to a solution. Engage in a back-and-forth discussion with the user.
*   **Never Assume:** If a requirement is vague, **ASK**. (e.g., User says "Make it blink" -> Agent asks "How fast? What color? Blocking or non-blocking? Under what conditions?").
*   **Challenge Context:** If a request violates the established architecture (e.g., global state vs. Pure State), strictly challenge it and propose the architectural alternative.
*   **Output:** Produce Markdown files (`.md`) and Mermaid diagrams. Code should rarely be written during this phase.

## 3. Role: The Programmer (Implementer)
When writing code:

### Architecture: Pure State Pattern
*   **Logic:** Resides strictly in `GamePhase` classes.
*   **Context:** The `Game` class is a minimal Context that owns data (`GameState`) and hardware.
*   **Memory Rule:** Strictly **NO Dynamic Allocation** (`new`, `malloc`) for phases or core logic. Use the `PhasePool` via `game.getPhase<T>()`.
    *   **Guidance on Large Objects:** Do not dynamically allocate large objects like `GameState`.
    *   **String/State Caching:** Avoid copying large strings (like C-style `strncpy` arrays) in UI component state caches. Instead, reference pointers to `std::string` objects already defined in long-lived state (e.g., `Player` objects) whenever possible.
*   **Hardware Ownership:** The `Game` class owns the hardware. Phases only *borrow* it. Only `Game::setup()` initializes hardware.

### Commenting Style
*   **General:** Prioritize self-documenting code. Use comments to explain *Why*, not *What*.
*   **Classes:** Every class file (header) **MUST** have a top-level block comment describing clearly **what it is** and **listing its specific responsibilities**.
*   **Tests:** Every test function **MUST** have a brief description explaining **what edge case it targets**. This prevents future agents from breaking the intent of the test.

### Workflow
1.  **Read Design:** Understand *exactly* what you are building.
2.  **No Magic Numbers:** Avoid hardcoded values. Define constants or macros with descriptive names (e.g., `#define BLINK_HALF_PERIOD 500`).
    *   **In Tests:** This rule is **MANDATORY** for test files. Never use literal values (e.g., `512`) in assertions; use a named constant (e.g., `ADC_VALUE_PLUS_500`) to explain the *intent* of the value.
    *   **Optimization Note:** Prefer Macros (`#define`) over const variables for constants to save memory on Arduino.
3.  **Small Methods & De-duplication:** Prefer small, single-responsibility methods. Actively refactor detected code duplication into helper methods.
4.  **Test First (Native):** Write a test in `src/farkle/test/` that fails.
5.  **Implement:** Write the minimal code to pass the test.
6.  **Refactor & Verify:** Ensure style matches existing code.
7.  **Update Design:** If you changed an interface, update the `.md` file immediately.

### Tooling & Environment
*   **Root:** Project root is `./`. Source root is `src/farkle/`.
*   **Build:** `cd src/farkle && pio run -e esp32dev`
*   **Testing:** `cd src/farkle && pio test -e native`
*   **Mocking:** We use **Native Mocks** via link-time substitution (see `src/farkle/test/mocks/`). **Do NOT** install external mocking frameworks.

## 4. Testing Strategy (The Pyramid)
See `design/TESTING_STRATEGY.md` for details.

### Tier 1: Game Logic (The Brain)
*   **Command:** `pio test -e native`
*   **Scope:** Verifies the State Machine, Transitions, and Game Rules.
*   **Mocking:** We mock the **Components** (e.g., `ScoreDisplay` is a fake class).
*   **Categories:**
    *   *Small:* Logic & Math.
    *   *Medium:* Turn Lifecycle.
    *   *Large:* Full Game.

### Tier 2: Component Logic (The Body)
*   **Command:** `pio test -e component_tests`
*   **Scope:** Verifies the internal logic of hardware drivers (e.g., `ScoreDisplay.cpp`).
*   **Mocking:** We compile the **REAL** component code but mock the **External Libraries** (e.g., `LedControl`, `FastLED`).
*   **Goal:** Ensure low-level formatting (like splitting numbers into digits) and performance are correct without needing physical hardware.

## 5. Git & Commit Etiquette
*   **Atomic Commits:** Use semantic messages (`feat:`, `fix:`, `refactor:`, `docs:`).
*   **Rebasing:** If a rebase is required, remember to `git add` changes before `git rebase --continue`.
*   **Force Push:** If you rebased a shared branch, use `git push --force-with-lease`.
