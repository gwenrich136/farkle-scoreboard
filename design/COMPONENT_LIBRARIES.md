> **Scope:** Defines the API and behavior of all custom hardware component libraries, such as `ControlPad`, `ScoreDisplay`, and `LedProgressGrid`.
> **Status:** **LIVE DOCUMENT** - This file represents the current source of truth. If code changes, this document MUST be updated.

# Component Library Designs for Farkle Scoreboard

This document outlines the design and intended functionality of the custom component libraries developed for the Farkle Scoreboard project. Each section details the purpose, API, and key behaviors of a specific component, reflecting the discussions and decisions made during the design phase.

---

## 1. ControlPad

### Purpose
The `ControlPad` component is responsible for managing both digital and analog inputs, translating them into logical game actions and navigation. It handles debouncing, analog ladder stability, and high-precision encoder rotation.

### API Design
-   **`ControlPad()`**: Constructor.
-   **`GameInput read()`**: Scans all inputs (Digital pins, Analog ladder, and Encoder buffer). Returns a `GameInput` struct containing the current discrete `ButtonAction` and any `rotationDelta` from the encoder.
    -   **Digital Priority**: If a digital button (BANK, FARKLE, SELECT) is pressed simultaneously with an analog ladder button (PLUS_50, etc.), the digital action takes priority.
    -   **Single Action per Press**: Discrete buttons return their action only once per distinct press/release cycle (no auto-repeat).
-   **`bool isToggled()`**: Returns the current state of the latching switch (A3), used for system-wide mode toggles (e.g., "Total Score" view).

### Key Logic & Behavior
-   **Hybrid Input Model**:
    -   **Digital (D4, D5, D6)**: High-reliability interrupts/polling for core actions (SELECT, BANK, FARKLE).
    -   **Analog Ladder (A2)**: Interprets voltage levels to trigger CLEAR, PLUS_50, PLUS_100, or PLUS_500. Requires a **50ms stability window** to filter NeoPixel power noise.
-   **Encoder Rotation (D2, D3)**:
    -   Uses **Interrupts (ISR)** to ensure no pulses are missed during display updates.
    -   Maintains an **Atomic Rotation Buffer** that tracks net displacement (ticks).
    -   The `read()` method consumes this buffer, populating `GameInput.rotationDelta` and resetting the buffer to zero.
-   **Input Data Structures**: The `GameInput` struct and `ButtonAction` enum are defined in `src/farkle/include/Input.h`.

---

## 2. FarkleWarningLights

### Purpose
The `FarkleWarningLights` component provides a system-wide visual map of every player's "farkle" status using an 8-LED NeoPixel strip. It serves both as a "turn pointer" and a danger indicator.

### API Design
-   **`FarkleWarningLights(int pin)`**: Constructor. Initializes the component for an 8-LED strip (NeoPixel) on the specified digital pin.
-   **`void update(const int* farkleCounts, int playerCount, int blinkingPlayerIndex, bool isBlinking)`**: Updates the entire strip.
    -   `farkleCounts`: An array of the current farkle count for every player in the game.
    -   `playerCount`: Total number of active players.
    -   `blinkingPlayerIndex`: The index of the player who should receive the blinking "turn indicator" treatment. Pass `-1` if no player should blink (e.g., during banking or farkling animations).
    -   `isBlinking`: A flag (synced with the 500ms global timer) that toggles the blinking player's LED.
-   **`void alternate(int currentPlayerIndex)`**: Triggers the alternating Yellow/Red "Pain" animation for the specified player (used during `PenaltyFarklingPhase`).

### Key Logic & Behavior
-   **Visual Hierarchy**:
    -   **Blinking Player (Active/Flashing)**: If `blinkingPlayerIndex` is valid, that player's LED flashes at **50% Brightness** (128).
    -   **Other Players (Idle/Solid)**: LEDs for other players are **Solid** and at **50% Brightness** (128).
    -   **Pain Animation (Alternate)**: When `alternate()` is called (e.g., during `PenaltyFarklingPhase`), the LEDs operate at **Full Brightness** (255) to maximize the "alarm" effect.
    -   **Global Brightness**: Generally, active LEDs use 50% brightness (128) to avoid being overpowering, with the exception of the "Pain" animation.
-   **Color Logic**:
    -   **0 Farkles**: **White** (Blinking player only; Idle/Solid players are **OFF**).
    -   **1 Farkle**: **Yellow** (Warning).
    -   **2+ Farkles**: **Red** (Danger).
-   **Hardware Optimization**: The component tracks the previous state and only calls `pixels.show()` when a value, current player, or blink state has changed.
-   **Safety**: The strip is automatically cleared during `reset()`.

---

## 3. LedProgressGrid

### Purpose
The `LedProgressGrid` component manages an 8x8 NeoPixel grid to display player scores as progress bars. It is a stateful UI component that handles player configuration, color assignments, custom row layouts, and animated effects like blinking.

### API Design

#### Setup & State Management
-   **`LedProgressGrid(uint8_t pin)`**: Constructor. Initializes the NeoPixel strip for a hardcoded 8x8 grid (64 pixels).
-   **`void reset()`**: Resets the component to its initial state, clearing all player configurations and turning off all LEDs, preparing for a new game.
-   **`int addPlayer(uint16_t hue)`**: Dynamically adds a player to the grid configuration.
    -   Assigns the given `hue` to the new player. The `GameState` is responsible for generating and providing this color.
    -   Returns the `playerIndex` (0-indexed) of the newly added player.

#### Display Modes

-   **`void displayPlayersPregame(std::optional<uint16_t> pendingPlayerHue)`**: Displays the current player setup in a pre-game or player selection screen.
    -   Illuminates all rows assigned to each existing player with their solid color.
    -   If `pendingPlayerHue` has a value: The rows that will be assigned to the *next* player will blink with the provided prospective hue.

-   **`void update(const std::vector<int>& scores, int currentPlayerIndex, int atRiskScore)`**: The primary method for rendering game scores during active gameplay. This should be called repeatedly in the main game loop.
    -   The `LedProgressGrid` internally calculates `maxScore` as the maximum of the `targetScore` and the highest potential score (banked + at-risk) among all players.
    -   Displays `scores[playerIndex]` as solid progress bars for all players, using their assigned colors. The progress bar fills **uniformly across all assigned rows simultaneously**. For example, if a player is assigned 2 rows and has 50% of the max score, both rows should be 50% illuminated, rather than one row being full and the other empty.
    -   For the `currentPlayerIndex`:
        -   Their `scores[currentPlayerIndex]` (banked score) is shown as solid.
        -   Their `atRiskScore` is shown as a blinking extension to their progress bar, using half brightness and a 500ms on/off cycle.

-   **`void clear()`**: Turns off all pixels on the grid.
-   **`int getMaxScore()`**: Returns the current internal `maxScore` used for scaling. Primarily used for testing.

### Key Logic & Behavior
-   **Player-Agnostic Core:** The `update()` method is designed to be called with game state, while player-specific configurations (hues, row mapping) are managed internally.
-   **Dynamic Hue Assignment:** Colors are automatically generated and assigned using the golden ratio to ensure good visual distinction.
-   **Custom Row Mapping:** The component internally handles specific row assignments based on the total number of players:
    -   **1 Player:** Uses rows 2, 3, 4, 5.
    -   **2 Players:** P1 uses rows 0, 1, 2; P2 uses rows 5, 6, 7.
    -   **3 Players:** P1 uses rows 0, 1; P2 uses rows 3, 4; P3 uses rows 6, 7.
    -   **4 Players:** P1 uses rows 0, 1; P2 uses rows 2, 3; P3 uses rows 4, 5; P4 uses rows 6, 7.
    -   **5-8 Players:** Each player gets one row, starting with Player 1 at row 0.
-   **Snaking Pixel Layout:** The underlying NeoPixel hardware is assumed to be wired in a snaking pattern, which is handled by the `get_pixel_index` helper.
-   **Internal `maxScore` Calculation:** The `update` method calculates the effective `maxScore` for display scaling as `max(targetScore, max_potential_player_score)`. This ensures that the grid bounds expand when a player reaches or exceeds the target, providing clear visual feedback on the progress relative to the goal and the competition.
-   **Non-Linear Brightness (Gamma Correction):** To align with human visual perception, the "remainder" pixel (the partially lit LED at the end of a bar) uses a 4th-degree polynomial brightness curve ($brightness = (2.7x^4 - 3.15x^3 + 1.2x^2 + 0.25x) \times max\_brightness$). This curve ($y'(0)=0.25$, $y'(1)=4$) ensures a slow increase at the low end to allow distinguishing small values, while ramping up significantly at the high end to account for the fact that a noticeable difference requires a larger absolute change when brightness is already high.
-   **Blinking:** Blinking effects for at-risk scores and pending players are handled internally, using `millis()` for timing (500ms on/off cycle).
-   **Memory & Optimization:** To prevent unnecessary hardware communication, the component maintains a `State` "memory". It only calls `_pixels.show()` if the input state (scores, current player, at-risk score, or blink state) has changed since the last refresh. An internal `isDirty` flag within the `State` struct can be used to force a refresh after operations like `reset()` or `addPlayer()`.

---

## 4. ScoreDisplay

### Purpose
The `ScoreDisplay` component controls three 5-digit 7-segment displays (driven by MAX7219 chips) to show various numerical scores from the game.

### API Design
-   **`ScoreDisplay(int csPin)`**: Constructor. Initializes the hardware SPI directly. It uses the default SPI MOSI (D11) and SCK (D13) pins.
-   **`void addDisplay(DisplayType type, int deviceIndex)`**: Maps a logical `DisplayType` to a physical `deviceIndex`.
-   **`void print_number(int number, DisplayType type, bool blink = false)`**: Displays an integer `number` on the display mapped to the given `DisplayType`.
    -   The number will be right-aligned on the 5-digit display.
    -   If `blink` is `true`, the display's intensity will alternate between LOW (2) and HIGH (12) periodically.
-   **`void clear(DisplayType type)`**: Clears all digits on the display mapped to the given `DisplayType`.

### Key Logic & Behavior
-   **Shared SPI Bus**: To support the high-speed requirements of the IPS LCD, the `ScoreDisplay` is migrated to the hardware SPI bus (D11/D13). It shares these pins with the `TextDisplayV2` but is controlled by its dedicated **CS (D10)** line.
-   **Three Dedicated Displays:** The component provides three independent 5-digit displays, logically identified by the `DisplayType` enum:
    -   `AT_RISK_SCORE`
    -   `CURRENT_PLAYER_SCORE`
    -   `COMPETITION_SCORE`
-   **Score Overflow Handling:** If the input `number` exceeds 99,999, the display will show `99999`.
-   **Blinking Capability:** When enabled via `print_number`, the component uses `millis()` to toggle the device's intensity between two levels (4 and 10 on a 0-15 scale) every 500ms. This is non-blocking and relies on `print_number` being called frequently in the main game loop to update the intensity state.
-   **Memory-Efficient Implementation:** The digit extraction and formatting are implemented using stack-allocated character buffers and integer arithmetic to avoid dynamic memory allocation and `std::string` overhead on the embedded target.
- **Memory & Optimization:** To prevent unnecessary hardware communication, the component maintains a `State` "memory" for each of the three displays. It only writes to MAX7219 registers if the requested state (number, blink mode, or calculated intensity) has changed since the last update.

---

## 5. TextDisplay

### Purpose
The `TextDisplay` component acts as a versatile UI manager for the SH1106 128x64 OLED display, providing distinct display modes for various in-game messages and interactive screens.

### API Design

#### Static Text Display Modes
-   **`void print(const char* message)`**: Displays a single message centered on the screen.
-   **`void displayTitle(const char* title)`**: Displays a single line of `title` text, centered horizontally and vertically, using a default large font.
-   **`void displayTitleWithSubtitle(const char* title, const char* subtitle)`**: Displays a main `title` centered towards the top, with a smaller `subtitle` centered towards the bottom.
-   **`void displayTitleWithSubtitles(const char* title, const char* leftSubtitle, const char* rightSubtitle)`**: Displays a main `title` centered towards the top, with two smaller subtitles at the bottom: one left-aligned (`leftSubtitle`) and one right-aligned (`rightSubtitle`).

#### Animated Scrolling Mode (Future/TODO)
-   **`void setScrollingMessage(const char* message)`**: Sets the text to be scrolled.
-   **`void updateScrollingMessage()`**: To be called repeatedly in the main loop to animate a vertical (Star Wars-style) scroll of the message. (Currently a **TODO** for future implementation).

#### Interactive UI Modes
-   **`void printSelectionScreen(const char* selectionTitle, const char* selectionItem)`**: Renders an interactive selection screen.
    -   Displays `selectionTitle` centered near the top using a smaller font.
    -   Displays `selectionItem` centered below using a larger font.
    -   Draws up/down arrows above and below the `selectionItem` to indicate that the value can be changed.

-   **`void displayCharacterInput(const char* currentName, int activeIndex)`**: Renders an interactive screen for character-by-character name input.
    -   The character at `currentName[activeIndex]` is displayed large and centered, with "carrot-like" up/down arrows above and below it (to change the character).
    -   The rest of the `currentName` string is drawn in a smaller font to the left and right of the active character.
    -   As `activeIndex` changes (controlled by left/right buttons in the main game loop), the background string appears to slide, keeping the active character centered.
    -   Player names have a maximum length (e.g., 12 characters), but the display can handle names that temporarily extend beyond the screen width, which will scroll into view as `activeIndex` changes.

### Key Logic & Behavior
-   **Multi-Page Rendering**: The SH1106 display uses a page-buffered approach. The `do...while` loops in the drawing methods ensure that the entire screen is rendered correctly by running all drawing commands for each vertical "page" of the display.
-   **Deterministic Layout**: To prevent visual jitter or "shimmering" on the I2C OLED, all coordinates (x, y) are pre-calculated once per frame **before** entering the `do...while` page loop. This ensures that every page of the buffer is rendered with identical, stable coordinates.
-   **Vertical Alignment**: The component uses `setFontPosTop()` and fixed font height constants (e.g., `TEXT_DISPLAY_MAIN_HEIGHT`) to ensure that vertical positioning is absolute and independent of character-specific metrics (like descenders in 'y' or 'g'). This prevents text from "jumping" when the content changes.
-   **State Caching**: The component caches the last rendered content and current mode using `std::string` comparison to prevent unnecessary screen refreshes, optimizing I2C bus usage.
-   **I2C Communication**: Uses an SH1106 128x64 OLED display via I2C, with a confirmed address of `0x3C`.
-   **Visual Styling**: Uses specific fonts for titles and main text to ensure hierarchy and readability.

---

## 6. TextDisplayV2 (IPS LCD)

### Purpose
The `TextDisplayV2` component is a high-performance UI manager for the **ST7789 240x240 Color IPS LCD**. It is designed to be functionally equivalent to the original `TextDisplay` but utilizes the hardware SPI bus and is optimized for the larger color-capable resolution.

### API Design

#### Static Text Display Modes
-   **`void print(const char* message, uint16_t hue = 0xFFFF)`**: Displays a single message centered on the 240x240 screen, optionally rendering the text in a specific `hue`.
-   **`void displayTitle(const char* title)`**: Displays a single line of `title` text, centered horizontally and vertically.
-   **`void displayTitleWithSubtitle(const char* title, const char* subtitle)`**: Displays a main `title` centered towards the top, with a smaller `subtitle` centered towards the bottom.
-   **`void displayTitleWithSubtitles(const char* title, const char* leftSubtitle, const char* rightSubtitle)`**: Displays a main `title` centered towards the top, with two smaller subtitles at the bottom: one left-aligned (`leftSubtitle`) and one right-aligned (`rightSubtitle`).

#### Interactive UI Modes
-   **`void printSelectionScreen(const char* selectionTitle, const char* selectionItem, uint16_t hue = 0xFFFF)`**: Renders an interactive selection screen. The `selectionItem` can be rendered in a specific `hue` (defaulting to White) to highlight player identities during setup.
-   **`void printHeadToHeadScreen(const char* p1Place, const char* p1Name, uint16_t p1Hue, const char* p2Place, const char* p2Name, uint16_t p2Hue)`**: Renders a split-screen "Leaderboard" or "Head-to-Head" display used during in-game phases. Displays the current player (P1) on the top half and the competitor/leader (P2) on the bottom half, including rank ordinal strings (e.g., "1st", "2nd") aligned to the left edge of the player names.
-   **`void displayCharacterInput(const char* currentName, int activeIndex, uint16_t hue = 0xFFFF)`**: Renders an interactive screen for name input, with the active character highlighted in the provided `hue`.

### Key Logic & Behavior
-   **Unified Color Identity**: By accepting `uint16_t` hue values, the LCD can match the text and UI elements to the specific hue assigned to the current player on the LED grid, directly mapping standard color values to RGB565.
-   **Hardware SPI Bus**: Uses the Uno R4's hardware SPI (D11/D13) for zero-latency UI updates. It shares this bus with the `ScoreDisplay` (MAX7219) but uses a dedicated **CS (A4)** pin.
-   **Control Pins**:
    -   **CS (A4)**: Chip Select.
    -   **DC (A5)**: Data/Command.
    -   **RES (D7)**: Hardware Reset.
    -   **BLK (D8)**: Backlight Control (automatically set to 100% in `begin()`).
- **Resolution (240x240)**: Layouts are optimized for the high-resolution square format.
- **Color Defaults**: To ensure compatibility during migration, text defaults to **White (0xFFFF)** on a **Black (0x0000)** background.
- **State Caching**: The component caches the last rendered content to skip redundant SPI updates.

---

## 7. MemoryCard

### Purpose
The `MemoryCard` component provides a high-level, game-aware interface for the SD card. It manages the global player name pool, tracks multiple game sessions (partial and completed), and provides "Live Snapshot" capabilities for game recovery and previews.

### API Design

#### Initialization & Pool Management
- **`bool begin()`**: Initializes the SD card hardware (SPI) and verifies/creates the required file tree (`/sys`, `/partial`, `/completed`).
    - Reserves a fixed-size memory allocation (buffer) for up to 50 players to hold their `Name` (max 12 chars), `PlayerState` enum (`AVAILABLE`, `UNUSED`, `SELECTED`, `DELETED`), and `Frequency` (integer).
    - Reads the `players.csv` pool into this memory and self-sorts the pool in descending order based on frequency.
- **`bool addName(const char* name)`**: Appends a new name to the first `UNUSED` slot in the internal pool, setting its state to `AVAILABLE` and frequency to 0. Checks for uniqueness.
- **`bool deleteName(int poolIndex)`**: Marks a player at the given index as `DELETED`.

#### Selection Helpers
- **`const char* reservePlayer(int poolIndex)`**: Marks the player at the given index as `SELECTED` and returns a pointer to their name string in the MemoryCard's allocation. The calling game logic must immediately copy this string into its own fixed `char[13]` allocation, as the MemoryCard's allocation will be cleared once the game starts.
- **`void unreservePlayer(int poolIndex)`**: Marks a previously `SELECTED` player back to `AVAILABLE`.
- **`int getNextAvailableIndex(int currentCursor)`**: A helper for the `PlayerSelectionPhase`. Increments the cursor, skipping any indices that are not explicitly marked as `AVAILABLE` (e.g., skips `SELECTED`, `DELETED`, or `UNUSED`).
- **`int getPrevAvailableIndex(int currentCursor)`**: Decrements the cursor, skipping indices not marked as `AVAILABLE`.
- **`void finalizeSelection()`**: Called at the end of the `PlayerSelectionPhase`.
    - Iterates through the memory pool and collects all names marked `AVAILABLE` and `SELECTED` (skipping `DELETED` and `UNUSED`).
    - Increments the frequency counter for all `SELECTED` players.
    - Rewrites `players.csv` from scratch with the retained players, keeping them in sorted order (descending by frequency).

#### Game Session Management
- **`struct UndoResult { uint8_t playerIndex; int score; uint8_t farkles; }`**: Data returned when a turn is reverted.
- **`uint32_t startNewGame(int targetScore, const std::vector<Player>& players)`**: Creates a new ID-based folder in `/partial/`, writes the `meta.csv`, and creates `sys/active_id.txt`. Returns the new Game ID.
- **`void logTurn(uint8_t playerIndex, int score, uint8_t farkleCount, bool finalRound, bool penalty)`**: Appends a 32-bit packed record to the `journal.bin` of the active game.
- **`UndoResult undoLastTurn()`**: Reverts the last record in `journal.bin` and returns the player's previous state (via "Scan Back" logic).
- **`void finalizeGame()`**: Moves the active game folder from `/partial/` to `/completed/` and deletes `sys/active_id.txt`.

#### Recovery & Preview
- **`std::vector<uint32_t> getPartialGameIds()`**: Returns a list of IDs currently in the `/partial/` directory.
- **`bool getSnapshot(uint32_t gameId, GameState& outState)`**: A "Quick Read" that populates a `GameState` object with the target score, players, their current scores, and the `currentPlayerIndex` by reading `meta.csv` and the end of `journal.bin`.

### Key Logic & Behavior
- **32-Bit Journaling**: Uses a fixed-width binary format for turn logs. This is more compact than CSV and allows for deterministic "Undo" and "Recovery" by jumping to specific offsets in the file.
- **Fixed Memory Allocation & String Ownership**: To ensure the UI remains responsive during scrolling, global player names and metadata are loaded into a fixed-size RAM buffer at boot. The `MemoryCard` strictly owns these strings during the pre-game phases. Once selection is complete, the `Game` must copy the selected names into its own storage, as the `MemoryCard`'s RAM buffer will be wiped when the main game starts to conserve memory.
- **Self-Sorting Roster**: Players are automatically sorted by play frequency (highest first) when loaded from `players.csv`, reducing the amount of scrolling needed for common players.
- **State-Based Selection**: The selection API uses a `PlayerState` enum to manage selection instead of maintaining a separate list of roster indices. This simplifies the scrolling logic to just skip any slot not marked `AVAILABLE`.
- **Self-Healing**: The component automatically reconstructs the `next_id.txt` by scanning the file system if system files are corrupted or missing.
- **Power-Loss Recovery**: By logging at the end of every turn (`EndOfTurnPhase`), the game minimizes data loss to at most the current turn's unsaved progress.

