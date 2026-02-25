# Hardware Upgrade Migration Plan: Version 1.0 to 2.0

> **Scope:** This document serves as the roadmap for the physical and logical transition from the V1 hardware (discrete LEDs, 7 buttons, basic speaker) to the V2 hardware (Status Strip, Rotary Encoder, SD Card, MP3 Player, and Latching Switch).
> **Status:** **LIVE STRATEGY** - This document tracks the active migration.

---

## 1. The Vision: Hardware v2 Experience

The goal of this upgrade is to transform the Farkle Scoreboard from a simple point-tracker into a professional-grade, interactive gaming device. 

### 1.1 The "Competitor Preview" Window
The **Rotary Encoder** (infinite) allows players to actively "scout" the competition without losing focus on their own turn. 
*   **Tactile Navigation:** Moving the encoder cycles through other players on the `COMPETITION_SCORE` display.
*   **Smart Selection:** At the start of a turn, the display defaults to the **Leader** (if the current player is not leading) or the **2nd Place Player** (to show the "person to beat").
*   **Global Alerts:** The display continues to blink if *any* player has triggered the final round, serving as a persistent warning that the "bell lap" is active.

### 1.2 The Status Strip & Turn Pointer
The **8-LED NeoPixel Strip** replaces the primitive 2-LED system with a dedicated indicator for every player row.
*   **Turn Awareness:** The LED corresponding to the active player's row **flashes** in sync with the scoring display.
*   **Strategic Farkle Tracking:**
    *   **Current Player (Flashing):** **White** (0 farkles), **Yellow** (1), **Red** (2+).
    *   **Other Players (Solid, Half-Brightness):** **OFF** (0 farkles), **Yellow** (1), **Red** (2+). This provides a permanent, low-profile map of everyone's "danger level."

### 1.3 Real-Time Strategy Toggle
A **Latching Switch** provides instant toggling between "Banked Score" and "Total Score" (Banked + At-Risk). This eliminates mental math for the player, showing them exactly where they would stand if they banked *now*.

---

## 2. The Target State: Hardware v2 Pin Map

| Pin | Component | Function | Status |
| :--- | :--- | :--- | :--- |
| **D0** | **Serial1 RX** | **MP3 Player TX** (Hardware UART) | Pending |
| **D1** | **Serial1 TX** | **MP3 Player RX** (Hardware UART) | Pending |
| **D2** | **Rotary Encoder A** | **Interrupt 0** (Smooth Scroll) | Pending |
| **D3** | **Rotary Encoder B** | **Interrupt 1** (Smooth Scroll) | Pending |
| **D4** | **SD Card CS** | **SPI Chip Select** (Data) | Pending |
| **D5** | **Button 1 (+50)** | Scoring Input | **Active (V1)** |
| **D6** | **Button 2 (+100)** | Scoring Input | **Active (V1)** |
| **D7** | **Button 3 (+500)** | Scoring Input | **Active (V1)** |
| **D8** | **Button 4 (BANK)** | Main Action | **Active (V1)** |
| **D9** | **Button 5 (FARKLE)** | Main Action | **Active (V1)** |
| **D10** | **MAX7219 CS** | **SPI Chip Select** (Displays) | **Active (V1)** |
| **D11** | **SPI COPI/MOSI** | Shared SPI Data | **Active (V1)** |
| **D12** | **SPI CIPO/MISO** | Shared SPI Data | **Active (V1)** |
| **D13** | **SPI SCK** | Shared SPI Clock | **Active (V1)** |
| **A0** | **LedProgressGrid** | NeoPixel Data (8x8) | **Active (V1)** |
| **A1** | **Status Strip** | NeoPixel Data (8-LED) | **DONE (V2)** |
| **A2** | **Button 6 (CLEAR)** | Reset Input | **Active (V1)** |
| **A3** | **Latching Switch** | **"Total Score" Toggle** | Pending |
| **A4** | **I2C SDA** | OLED Data | **Active (V1)** |
| **A5** | **I2C SCL** | OLED Clock | **Active (V1)** |

---

## 3. Migration Steps (The "Safe Point" Approach)

### **Phase 1: Visual Feedback Overhaul**
**Step 1: The Status Strip (A1) [DONE]**
*   **Hardware:** Unwire 2 LEDs. Connect 8-LED Status Strip to **A1**.
*   **Software:** 
    *   Overhaul `FarkleWarningLights` to manage an `Adafruit_NeoPixel` object.
    *   Implement logic for White/Yellow/Red colors and the "Sync-Blink" vs "Solid-Dim" behaviors.
*   **Verification:** Verify all 8 indicators respond correctly to turn changes and farkle events across all players.

### **Phase 2: Input & Navigation Foundation**
**Step 2: The ControlPad v2 (Encoder & Switch)**
*   **Hardware:** Connect **Rotary Encoder** (D2, D3), **Latching Switch** (A3), and rearrange **6 Buttons**. Remove +1000 button.
*   **Software:**
    *   Integrate Switch and Encoder into the `ControlPad` HAL.
    *   Implement `n-click` thresholding for encoder deltas.
*   **Verification:** Test discrete button presses, switch state reading, and smooth encoder deltas.

**Step 3: Menu Navigation (Pre-Game)**
*   **Software:** Update `TargetScoreSelectionPhase` and `PlayerSelectionPhase` to use `encoder.getDelta()` for scrolling.
*   **Verification:** Verify infinite, tactile scrolling through setup menus.

### **Phase 4: Logic & Strategy Enhancements**
**Step 4: The "Total Score" Toggle**
*   **Software:** Modify `InGamePhase` to check the `ControlPad` switch state and toggle the middle display calculation.
*   **Verification:** Verify real-time display switching during an active turn.

**Step 5: Competitor Preview (The Strategy Window)**
*   **Software:**
    *   Update `WaitingPhase` to use `encoder.getDelta()` to cycle through competitors.
    *   Implement "Skip Self" logic and "Reset on Turn End" behavior.
*   **Verification:** Verify that the player can check other scores without losing focus on their turn.

### **Phase 5: Persistence & Atmosphere**
**Step 6: SD Card Storage**
*   **Hardware:** Connect SD Module to SPI bus.
*   **Software:** Integrate `SdFat.h`. Implement `Storage` class for game-data persistence.
*   **Verification:** Verify that player names and target scores persist across power-cycles.

**Step 7: MP3 Audio**
*   **Hardware:** Connect DFPlayer Mini to **Serial1**.
*   **Software:** Implement `AudioManager` and trigger tracks for key game events.
*   **Verification:** Verify audio sync with game animations.

---

## 4. Rollout Strategy
1.  **Iterative Updates:** The `SCHEMATIC_AND_HARDWARE_GUIDE.md` will be updated after *each* step to reflect the current physical state.
2.  **Continuous Testing:** No step is considered complete until all unit and component tests pass.
