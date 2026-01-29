# Technical Design: Game State Machine

## 1. Problem Statement

The Farkle Scoreboard requires a formal system to manage the flow of the game. As the game's complexity grows with different user interactions (pre-game setup, in-game scoring, animations, post-game replays), an ad-hoc approach using simple `if/else` blocks or flags in the main loop will become unmanageable, bug-prone, and difficult to extend.

This document specifies a robust and scalable state machine architecture that provides a clear structure for game logic, ensures state transitions are handled safely, and allows for new features and game phases to be added in a clean, modular way.

## 2. Feature Description

This feature implements the central "brain" of the game, managing its state and flow. For the initial V1 implementation, this state machine will manage a simplified game flow:

1.  **Game Start:** The game begins immediately upon startup with a hardcoded 4-player game (named "Player 1", "Player 2", etc.). There is no pre-game menu.
2.  **Player Turn (`WAITING` phase):** The game waits for the current player to input scores from their dice rolls using the directional buttons. The `atRiskScore` accumulates with each button press.
3.  **Bank (`BANKING` phase):** When the player presses the `BANK` button, the game enters an uninterruptible animation phase. The `atRiskScore` will appear to slowly drain to zero while the player's banked score ticks up. Once the animation is complete, play passes to the next player, and the game returns to the `WAITING` phase.
4.  **Farkle (`FARKLING` phase):** When the player presses the `FARKLE` button, the game enters another uninterruptible animation phase where the `atRiskScore` slowly drains to zero, signifying the loss of those points. Play then passes to the next player, returning to the `WAITING` phase.
5.  **Game End (`PostGamePhase_V1` - Final Round Logic):** A "final round" is triggered the first time any player's banked score meets or exceeds **5000 points**. The game does *not* end immediately. Play continues, allowing all other players a chance to catch up. The game concludes and transitions to the `PostGamePhase_V1` if, at the *start* of a player's turn, `finalRoundTriggered` is true and that player's score is already 5000 or more. In the `PostGamePhase_V1` state, the final scores are displayed, but all input is ignored. This serves as a placeholder for the full post-game experience.

All animations will be time-based, ensuring a smooth and consistent user experience regardless of processor load.

## 3. Technical Discussion

This section details the architectural decisions, alternatives considered, and final design choices for the state machine implementation.

### 3.1 Core Architecture: The "Object Pool" State Pattern

After considering multiple alternatives, we have chosen to implement the **State Design Pattern** using an **Object Pool**.

*   **Description:** This pattern involves defining an abstract base class (`GamePhase`) and a concrete child class for each state (`WaitingPhase`, `BankingPhase`, etc.). A main `Game` class holds a pointer to the `currentPhase`. Instead of dynamically creating (`new`) and destroying (`delete`) phase objects on each transition, we will pre-allocate one instance of every phase object at startup and store them in a "pool". State transitions are achieved by changing the `currentPhase` pointer to refer to a different object in the pool.
*   **Pros:**
    *   **Excellent Encapsulation:** All logic for a specific phase is contained entirely within its class.
    *   **High Scalability:** Adding a new game state is as simple as creating a new class, with minimal changes to existing code.
    *   **Memory Safe:** By pre-allocating all objects, we completely avoid dynamic memory allocation in the main loop, eliminating the risk of heap fragmentation during gameplay.
*   **Cons:**
    *   **Higher Upfront Memory:** All phase objects exist in memory for the lifetime of the application.
    *   **More Boilerplate:** Requires more initial setup (defining base classes, child classes, the pool) than simpler alternatives.

### 3.2 Alternative 1: `enum` with `switch` statement

*   **Description:** A single `enum` (`GamePhases`) would define the possible states. The main loop would contain a large `switch(currentPhase)` statement, with a `case` for each state containing its logic.
*   **Reason for Rejection:** While simple for V1, this approach scales poorly. The main loop function becomes a monolithic block of code that is difficult to read, maintain, and extend as the game's complexity grows towards our long-term vision.

### 3.3 Alternative 2: Dynamic State Pattern

*   **Description:** A classic implementation of the State pattern where a new phase object is created with `new` and the old one is destroyed with `delete` upon every state transition.
*   **Reason for Rejection:** While offering the same encapsulation benefits, the frequent calls to `new`/`delete` introduce the risk of heap fragmentation. In a long-running embedded application with future large-memory features (WiFi, SD card), this risk was deemed unacceptable.

### 3.4 Key Design Decisions

*   **Central `Game` Class:** We will use a main `Game` class to act as the central 'Context' for the state machine. It will own all game data (`GameState`), the pool of phase objects, and the hardware components, while the `GamePhase` classes will encapsulate the actual game logic. This improves organization, reduces the risk of unintended side-effects, and enhances testability for our long-term vision. The minor upfront effort of creating the class is outweighed by these benefits.
*   **Time-Based Animation Logic:** Animations will be driven by time elapsed (`deltaTime`) rather than by processor ticks. This ensures animations remain smooth.
    *   **Mechanism:** The main `Game::loop()` will calculate `deltaTime` on each frame and pass it to the `update()` method. Animation phases (like `BankingPhase`) will contain a floating-point `accumulator` variable. On each update, this accumulator is increased by `(SCORE_ANIMATION_SPEED * deltaTime)`. The integer portion of the accumulator is then moved from the `atRiskScore` to the `bankedScore`. This handles the conversion from a time-based float to integer-based scores cleanly. The edge case where `pointsToMove` exceeds `atRiskScore` will be handled by taking the minimum of the two.
*   **Phase-Local State Management and `onEnter()`:** The Object Pool pattern means phase objects are reused, not recreated. Therefore, any internal state they hold (like the animation accumulator) must be reset upon entry.
    *   **Discussion:** An alternative was considered where a common timestamp (`lastUpdateTime`) could be stored in the global `GameState`). However, this breaks down for state that is truly local to a phase (e.g., `scoreMoveAccumulator` is meaningless to `WaitingPhase`). Storing such state in `GameState` would break encapsulation and lead to state-reset bugs.
    *   **Decision:** Each phase is responsible for its own internal state. The `GamePhase` interface *must* include an `onEnter()` method. This method is called by the main `Game` engine immediately after a state transition, providing a clean entry point for each phase to reset its internal variables (e.g., `this->scoreMoveAccumulator = 0.0f;`).
*   **Inheritance Hierarchy:** To prepare for future scalability, we will implement a three-layer class hierarchy from the start:
    *   `GamePhase` (Abstract Base Class)
    *   `InGamePhase` (Intermediate Class inheriting from `GamePhase`)
    *   `WaitingPhase`, `BankingPhase`, etc. (Concrete classes inheriting from `InGamePhase`).
    This structure allows for common logic to be shared at the `InGamePhase` level (like the `display()` method). For V1, the `PostGamePhase_V1` will inherit directly from `GamePhase` and temporarily contain duplicated display code, which is an acceptable trade-off.

## 4. Implementation Plan

The following files will be created or modified to implement the Game State Machine, following a "Pure State" architectural pattern where the `Game` class acts as a generic engine and all game logic is encapsulated within the `GamePhase` classes.

#### New Files - Common Definitions
*   **`src/farkle/include/ButtonActions.h`**
    *   **Why:** Defines the `ButtonAction` enumeration used by the `Game` engine and its phases.
    *   **Implementation Details:**
        *   `enum ButtonAction { NONE, BANK, FARKLE, CLEAR, DOWN_50, LEFT_100, RIGHT_500, UP_1000 };`

*   **`src/farkle/include/Displays.h`**
    *   **Why:** To group all display component objects into a single parameter object (struct), simplifying method signatures.
    *   **Implementation Details:**
        *   `struct Displays`: Will contain references (`&`) to all display components: `ScoreDisplay& scoreDisplay;`, `LedProgressGrid& grid;`, `FarkleWarningLights& farkleLights;`, `TextDisplay& oled;`.

#### New Files - Data Structures
*   **`src/farkle/include/GameState.h`**
    *   **Why:** Defines the core data structures, keeping data separate from logic.
    *   **Implementation Details:**
        *   `struct Player`: Will contain `std::string name;`, `int score;`, `int farkle_count;`, and `std::vector<int> score_history;`.
        *   `struct GameState`: Will contain `std::vector<Player> players;`, `int atRiskScore;`, `int currentPlayerIndex;`, `bool finalRoundTriggered = false;`. For V1, the `players` vector will be initialized with 4 hardcoded `Player` instances.

#### New Files - Phase Interfaces
*   **`src/farkle/include/GamePhase.h`**
    *   **Why:** Defines the core interfaces for the State pattern. This is the contract all phases must adhere to.
    *   **Implementation Details:**
        *   Forward-declare `class Game;`.
        *   `class GamePhase` will be an abstract base class.
        *   It will define the pure virtual interface:
            *   `virtual void onEnter(GameState& state) = 0;`
            *   `virtual GamePhase* update(Game& game, GameState& state, ButtonAction action, unsigned long deltaTime) = 0;`
            *   `virtual void display(const GameState& state, const Displays& displays) = 0;`
        *   `class InGamePhase : public GamePhase` will be an intermediate class.
            *   It will provide a concrete, shared implementation of the `display()` method.
            *   It will provide a protected helper method `void endTurn(GameState& state);` which will increment the `currentPlayerIndex`.

#### New Files - Game Engine
*   **`src/farkle/include/Game.h`** & **`src/farkle/src/Game.cpp`**
    *   **Why:** Implements the main `Game` engine class, which acts as a pure state machine manager and context for the phases.
    *   **Implementation Details:**
        *   The header will define the `PhasePool` as a private struct containing one instance of each concrete phase.
        *   The class will own the `PhasePool`, the `GameState` object, `GamePhase* currentPhase`, and all hardware component classes.
        *   It will provide a public templated method for phases to acquire pointers to other phases: `template<typename T> T* getPhase();`.
        *   `Game::setup()` will initialize hardware, initialize the `GameState` with 4 players, and set the initial state: `currentPhase = &phasePool.waiting;`.
        *   `Game::loop()` will be a pure state machine engine, containing no game-specific logic:
            1.  Calculate `deltaTime`.
            2.  Read input from the `ControlPad`.
            3.  Construct the `Displays` struct.
            4.  `GamePhase* nextPhase = currentPhase->update(*this, state, action, deltaTime);`
            5.  `if (nextPhase != currentPhase) { currentPhase = nextPhase; currentPhase->onEnter(state); }`
            6.  `currentPhase->display(state, displays);`

#### New Files - Concrete Phase Implementations
A new directory `src/farkle/src/phases/` will be created to house these files.

*   **`src/farkle/include/phases/WaitingPhase.h`** & **`src/farkle/src/phases/WaitingPhase.cpp`**
    *   **Why:** Implements the main interactive phase and serves as the entry point for a player's turn.
    *   **Implementation Details:** The `update()` method will contain all logic for this phase:
        *   **Check for Game End:** The first action will be to check `if (state.finalRoundTriggered && state.players[state.currentPlayerIndex].score >= 5000)`. If true, it will immediately `return game.getPhase<PostGamePhase_V1>();`.
        *   **Handle Input:** A `switch(action)` block will handle `UP_1000`, `RIGHT_500`, etc., by modifying `state.atRiskScore`.
        *   **Handle Transitions:** If `BANK` is pressed, `return game.getPhase<BankingPhase>();`. If `FARKLE` is pressed, `return game.getPhase<FarklingPhase>();`.

*   **`src/farkle/include/phases/BankingPhase.h`** & **`src/farkle/src/phases/BankingPhase.cpp`**
    *   **Why:** Implements the banking animation and the end-of-turn logic that follows.
    *   **Implementation Details:** The `update()` method will ignore input and run its animation. When the animation completes, it will perform all necessary end-of-turn logic in this order:
        1.  Add `atRiskScore` to the current player's `score`.
        2.  Reset `atRiskScore` to 0.
        3.  **Check for Final Round Trigger:** Check if `!state.finalRoundTriggered` and if any player's score is now `>= 5000`. If so, set `state.finalRoundTriggered = true;`.
        4.  Call the shared helper `this->endTurn(state);` to advance the `currentPlayerIndex`.
        5.  `return game.getPhase<WaitingPhase>();` to complete the transition.

*   **`src/farkle/include/phases/FarklingPhase.h`** & **`src/farkle/src/phases/FarklingPhase.cpp`**
    *   **Why:** Implements the farkle animation and the end-of-turn logic.
    *   **Implementation Details:** Similar to `BankingPhase`, when its animation completes, it will:
        1.  Reset `atRiskScore` to 0.
        2.  Call `this->endTurn(state);` to advance the `currentPlayerIndex`.
        3.  `return game.getPhase<WaitingPhase>();`.

*   **`src/farkle/include/phases/PostGamePhase_V1.h`** & **`src/farkle/src/phases/PostGamePhase_V1.cpp`**
    *   **Why:** Implements the simplified "frozen" post-game phase for V1.
    *   **Implementation Details:** Its `update()` method will be empty and simply `return this;`, ignoring all input.

#### Modified Files
*   **`src/farkle/src/main.cpp`**
    *   **Why:** To delegate all responsibility to the new `Game` class.
    *   **Changes:** The file will be reduced to a minimal skeleton: instantiate a global `Game myGame;`, call `myGame.setup();` in `setup()`, and `myGame.loop();` in `loop()`.