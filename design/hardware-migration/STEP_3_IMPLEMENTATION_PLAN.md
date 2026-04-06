# Technical Directive: Farkle Scoreboard Hardware v2 Migration (Step 3)

## **Objective**
Migrate the UI from the monochrome SH1106 OLED (I2C) to the ST7789 240x240 IPS LCD (Hardware SPI). This includes refactoring color ownership to the `GameState`, implementing the new `TextDisplayV2` component, and integrating the shared Hardware SPI bus.

---

## **Task 1: Color Sovereignty Refactor**
**Goal:** Transfer color generation and ownership from the `LedProgressGrid` component to the `GameState` logic. This decouples visual identity from hardware-specific components.

### **1.1 Update `GameState` and `Player` Structures**
-   **File:** `src/farkle/include/GameState.h`
-   Add `uint16_t hue` (0-65535) to the `Player` struct.
-   Implement `uint16_t getNextPlayerHue(int playerCount)`.
    -   Formula: `(index * 40503) % 65536` (Approximation of the Golden Ratio hue jump for 16-bit space).
-   Update `GameState::reset()` to clear hue data.

### **1.2 Refactor `LedProgressGrid` API**
-   **Files:** `src/farkle/lib/components/LedProgressGrid/LedProgressGrid.h/cpp`
-   Change `addPlayer()` to `addPlayer(int playerIndex, uint16_t hue)`.
-   Update the internal buffer to convert the `hue` to RGB using `pixels.ColorHSV(hue, 255, 255)`.

### **1.3 Update `Game` Logic**
-   **File:** `src/farkle/src/Game.cpp`
-   In `Game::addPlayer(const std::string& name)`, calculate the hue using the new utility.
-   Assign the hue to the `Player` object in `state.players`.
-   Pass this hue to `grid.addPlayer(index, hue)`.

### **1.4 Update Mocks and Tests**
-   **Files:** `src/farkle/test/mocks/include/LedProgressGrid.h`, `src/farkle/test/test_component_led_progress_grid/test_led_progress_grid.cpp`
-   Update the `LedProgressGrid` mock to match the `addPlayer(int, uint16_t)` signature.
-   Update all component and logic tests to provide a dummy hue (e.g., `0`).
-   **Validation:** Run `./run_tests.sh` and ensure 100% pass rate.

---

## **Task 2: The IPS Engine (TextDisplayV2)**
**Goal:** Implement the `TextDisplayV2` library as a standalone component with full parity to V1, supporting the 240x240 resolution and color-coded rendering.

### **2.1 Implement `TextDisplayV2` Component**
-   **Files:** `src/farkle/lib/components/TextDisplay/TextDisplayV2.h/cpp`
-   **Library Dependency:** Use `Adafruit_GFX` and `Adafruit_ST7789`.
-   **Constructor:** `TextDisplayV2(int cs, int dc, int res, int blk)`.
-   **Method Parity:** Re-implement all methods from `TextDisplay` (`print`, `displayTitle`, `printSelectionScreen`, etc.).
-   **Color Support:** Update `printSelectionScreen` and `displayCharacterInput` to accept an optional `uint16_t hue = 0xFFFF` (where `0xFFFF` indicates default white text).
-   **Hardware Setup:** In `begin()`, handle the `D7 (RES)` reset sequence and set `D8 (BLK)` to `HIGH` (or use PWM for 100% brightness).
-   **Optimization:** Use `setFont()` (e.g., `FreeSans9pt7b`) for high-fidelity text. Implement basic state-caching (check if text changed before redraw).

### **2.2 Create `TextDisplayV2` Mocks**
-   **Files:** `src/farkle/test/mocks/include/TextDisplayV2.h`, `src/farkle/test/mocks/src/TextDisplayV2.cpp`
-   Create a Mock class that records calls to `print`, `displayTitle`, and `printSelectionScreen`.
-   Ensure the mock correctly tracks the `uint16_t hue` parameter.

### **2.3 Component Testing**
-   **File:** `src/farkle/test/test_component_text_display/test_TextDisplayV2.cpp`
-   Port existing tests from `test_TextDisplay.cpp` to verify that `TextDisplayV2` logic (text centering, state caching) works correctly in the mock environment.
-   **Validation:** Compile the project and run the new component tests.

---

## **Task 3: Hardware SPI Convergence (Integration)**
**Goal:** Finalize the migration by swapping V1 for V2 in the game engine and moving the `ScoreDisplay` to the hardware SPI bus.

### **3.1 Update `Displays` and `GamePhase`**
-   **Files:** `src/farkle/include/Displays.h`, `src/farkle/include/GamePhase.h`
-   Update `struct Displays` to hold a reference to `TextDisplayV2& textDisplay` instead of `TextDisplay& oled`.
-   Update `GamePhase::display()` documentation/comments to reflect that `GameState` now provides player hues for rendering.

### **3.2 Shared Hardware SPI Mapping**
-   **File:** `src/farkle/src/Game.cpp` (Constructor)
-   Update `ScoreDisplay` initialization: Use **D11 (MOSI)**, **D13 (SCK)**, and **D10 (CS)**.
-   Initialize `TextDisplayV2` using **A4 (CS)**, **A5 (DC)**, **D7 (RES)**, and **D8 (BLK)**.
-   Note: Since LCD CS is missing on hardware, ensure A4 is driven LOW or the library is configured for no-CS operation.

### **3.3 Update Phase Rendering**
-   **Files:** `src/farkle/src/phases/*.cpp`
-   Update all calls from `displays.oled` to `displays.textDisplay`.
-   In `PlayerSelectionPhase`, pass the hue for the pending player: `getNextPlayerHue(state.players.size())`.
-   In `WaitingPhase`, pass the current player's hue: `state.players[state.currentPlayerIndex].hue`.

### **3.4 Final Verification**
-   **Files:** `src/farkle/test/test_game_logic/**/*.cpp`
-   Update the large-scale game logic tests to use the `TextDisplayV2` mock.
-   **Validation:** Run `./run_tests.sh`. Ensure all logic tests pass, verifying that the game flow is intact despite the component swap.
-   **Build Check:** Ensure `pio run` (or equivalent) succeeds for the `uno_r4_wifi` target.

---

## **Technical Standards**
-   **No Magic Numbers:** All coordinates (centering at 120, 120) must use constants like `LCD_CENTER_X`.
-   **Surgical Edits:** Do not refactor unrelated logic in `BankingPhase` or `FarklingPhase`.
-   **Memory Awareness:** Avoid `std::string` in the `TextDisplayV2` render loop; use `const char*` and stack buffers.
