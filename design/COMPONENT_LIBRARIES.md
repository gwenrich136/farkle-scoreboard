# Component Library Designs for Farkle Scoreboard

This document outlines the design and intended functionality of the custom component libraries developed for the Farkle Scoreboard project. Each section details the purpose, API, and key behaviors of a specific component, reflecting the discussions and decisions made during the design phase.

---

## 1. ControlPad

### Purpose
The `ControlPad` component is responsible for managing physical button inputs, translating them into logical game actions. It handles common input issues such as debouncing and preventing multiple simultaneous presses from registering as valid input.

### API Design
-   **`ControlPad()`**: Constructor. Initializes the control pad.
-   **`void addButton(int pin, ButtonAction buttonAction)`**: Configures a physical `pin` to trigger a specific `ButtonAction`. The `ControlPad` internally enables `INPUT_PULLUP` for the given pin.
-   **`ButtonAction read()`**: Scans all configured buttons. Returns the `ButtonAction` of a *single, debounced* button press.
    -   If more than one button is pressed simultaneously, it returns `ButtonAction::NONE`.
    -   If the same button remains pressed, it returns `ButtonAction::NONE` after the initial press, preventing repeated actions.

### Key Logic & Behavior
-   **Single Action per Press:** The `read()` method ensures that only one `ButtonAction` is registered per distinct button press, making it ideal for a state-based game loop.
-   **Dynamic Mapping:** Buttons are mapped to actions at runtime, providing flexibility for different control layouts.
-   **`ButtonAction` Enum:** This enum, representing game-level actions, lives in its own dedicated header file: `src/farkle/include/ButtonActions.h`. This decouples the core game actions from the `ControlPad` hardware library. The `ControlPad`'s `read()` method should return values from this centrally-defined enum. The dual-purpose actions (`DOWN_50`, etc.) depend on the current game state for their interpretation.

---

## 2. FarkleWarningLights

### Purpose
The `FarkleWarningLights` component provides a simple visual indication of a player's "farkle" count during a turn using two LEDs (one yellow, one red).

### API Design
-   **`FarkleWarningLights(int yellowPin, int redPin)`**: Constructor. Initializes the component with the Arduino pins connected to the yellow and red LEDs, configuring them as outputs.
-   **`void farkle_state(int state)`**: Sets the state of the warning lights based on the integer `state`.
    -   `state = 0`: Both LEDs are off.
    -   `state = 1`: The yellow LED is on, red is off.
    -   `state = 2` (or greater): Both the yellow and red LEDs are on.

### Key Logic & Behavior
-   **Direct State Mapping:** The integer input directly maps to the visual output, simplifying usage in the main game logic.
-   **Max Farkle Count:** The game logic will ensure the `state` passed to `farkle_state()` will never exceed 2, as a third farkle resets the counter to zero.

---

## 3. LedProgressGrid

### Purpose
The `LedProgressGrid` component manages an 8x8 NeoPixel grid to display player scores as progress bars. It is a stateful UI component that handles player configuration, color assignments, custom row layouts, and animated effects like blinking.

### API Design

#### Setup & State Management
-   **`LedProgressGrid(uint8_t pin, uint16_t num_pixels)`**: Constructor. Initializes the NeoPixel strip.
-   **`void reset()`**: Resets the component to its initial state, clearing all player configurations and turning off all LEDs, preparing for a new game.
-   **`int addPlayer()`**: Dynamically adds a player to the grid configuration.
    -   Assigns a unique color (hue) to the new player. The first player gets a random hue, subsequent players get hues generated using the golden ratio for maximal distinction.
    -   Returns the `playerIndex` (0-indexed) of the newly added player.

#### Display Modes

-   **`void displayPlayersPregame(bool isPlayerPending)`**: Displays the current player setup in a pre-game or player selection screen.
    -   Illuminates all rows assigned to each existing player with their solid color.
    -   If `isPlayerPending` is `true`: The rows that would be assigned to the *next* player (Player `_playerCount`) will blink with their prospective hue. If no players are added yet (`_playerCount == 0`), the middle four rows will blink.

-   **`void update(const std::vector<int>& scores, int currentPlayerIndex, int atRiskScore)`**: The primary method for rendering game scores during active gameplay. This should be called repeatedly in the main game loop.
    -   The `LedProgressGrid` internally calculates `maxScore` based on the highest score provided (lowest multiple of 2000 greater than the highest score, with a minimum of 10,000).
    -   Displays `scores[playerIndex]` as solid progress bars for all players, using their assigned colors.
    -   For the `currentPlayerIndex`:
        -   Their `scores[currentPlayerIndex]` (banked score) is shown as solid.
        -   Their `atRiskScore` is shown as a blinking extension to their progress bar, using half brightness and a 500ms on/off cycle.

-   **`void clear()`**: Turns off all pixels on the grid.

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
-   **Internal `maxScore` Calculation:** The `update` method calculates the effective `maxScore` for display scaling based on game rules (multiples of 2000, min 10000).
-   **Blinking:** Blinking effects for at-risk scores and pending players are handled internally, using `millis()` for timing.

---

## 4. ScoreDisplay

### Purpose
The `ScoreDisplay` component controls three 5-digit 7-segment displays (driven by MAX7219 chips) to show various numerical scores from the game.

### API Design
-   **`ScoreDisplay(int dataPin, int clkPin, int csPin)`**: Constructor. Initializes the `LedControl` library with the appropriate pins for DIN, CLK, and CS, and performs basic setup for the three MAX7219 devices (wake up, set intensity, clear display).
-   **`void print_number(int number, int deviceIndex)`**: Displays an integer `number` on the specified `deviceIndex` (0, 1, or 2).
    -   The number will be right-aligned on the 5-digit display.

### Key Logic & Behavior
-   **Three Dedicated Displays:** The component provides three independent 5-digit displays, intended for:
    1.  `current_at_risk_score`
    2.  `current_player_banked_score`
    3.  `leading_score`
-   **Score Overflow Handling (Desired):** If the input `number` exceeds 99,999, the display should show `99999`. (The current implementation will be updated to reflect this desired behavior.)
-   **Readability-Focused Implementation:** The digit extraction and formatting are implemented using `std::string` for readability, with negligible performance impact on the target hardware.

---

## 5. TextDisplay

### Purpose
The `TextDisplay` component acts as a versatile UI manager for the SH1106 128x64 OLED display, providing distinct display modes for various in-game messages and interactive screens.

### API Design

#### Static Text Display Modes
-   **`void displayTitle(const char* title)`**: Displays a single line of `title` text, centered horizontally and vertically, using a default large font.
-   **`void displayTitleWithSubtitle(const char* title, const char* subtitle)`**: Displays a main `title` centered towards the top, with a smaller `subtitle` centered towards the bottom.
-   **`void displayTitleWithSubtitles(const char* title, const char* leftSubtitle, const char* rightSubtitle)`**: Displays a main `title` centered towards the top, with two smaller subtitles at the bottom: one left-aligned (`leftSubtitle`) and one right-aligned (`rightSubtitle`).

#### Animated Scrolling Mode (Future/TODO)
-   **`void setScrollingMessage(const char* message)`**: Sets the text to be scrolled.
-   **`void updateScrollingMessage()`**: To be called repeatedly in the main loop to animate a vertical (Star Wars-style) scroll of the message. (Currently a **TODO** for future implementation).

#### Interactive UI Modes
-   **`void displaySelector(const std::vector<const char*>& items, int selectedIndex)`**: Renders an interactive selection screen, such as for choosing a player name from a list.
    -   Displays `items[selectedIndex]` large and centered.
    -   Draws "carrot-like" up/down arrows above and below the selected item to indicate scrollability.
    -   The main game loop is responsible for updating `selectedIndex` based on user input.

-   **`void displayCharacterInput(const char* currentName, int activeIndex)`**: Renders an interactive screen for character-by-character name input.
    -   The character at `currentName[activeIndex]` is displayed large and centered, with "carrot-like" up/down arrows above and below it (to change the character).
    -   The rest of the `currentName` string is drawn in a smaller font to the left and right of the active character.
    -   As `activeIndex` changes (controlled by left/right buttons in the main game loop), the background string appears to slide, keeping the active character centered.
    -   Player names have a maximum length (e.g., 12 characters), but the display can handle names that temporarily extend beyond the screen width, which will scroll into view as `activeIndex` changes.

### Key Logic & Behavior
-   **Dynamic Centering:** All text display functions will dynamically calculate positioning using `U8g2lib` font metrics, ensuring proper alignment regardless of font or message length.
-   **Separation of Concerns:** For interactive modes, the `TextDisplay` is responsible solely for rendering. The game's main loop handles input and manages the state (`selectedIndex`, `currentName`, `activeIndex`).
-   **I2C Communication:** Uses an SH1106 128x64 OLED display via I2C, with a confirmed address of `0x3C`.
