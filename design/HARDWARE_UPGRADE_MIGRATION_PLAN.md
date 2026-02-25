# Hardware Upgrade Migration Plan: Version 2.5 (Hybrid Strategy)

> **Scope:** Roadmap for the V1 to V2 hardware transition, featuring the ST7789 Color IPS Display and a Hybrid Input Architecture.
> **Status:** **LIVE STRATEGY** - Step 1 is DONE. Step 2 is the active target.

---

## 1. The Vision: Hardware v2 Experience

The V2 upgrade transforms the Farkle Scoreboard into a premium, color-coded interactive device.

### 1.1 The IPS Color Display (ST7789)
A 240x240 RGB IPS display replaces the monochrome OLED.
*   **Color-Coded Feedback:** Player names, turn indicators, and "at-risk" scores are rendered in the player's unique Golden Ratio hue.
*   **High-Speed SPI:** Utilizing the Uno R4's hardware SPI bus for zero-latency UI updates.

### 1.2 The "Competitor Preview" Window
The **Rotary Encoder** (infinite) allows players to cycle through competitors on the `COMPETITION_SCORE` display.
*   **Smart Selection:** Defaults to the Leader or the 2nd Place Player at the start of a turn.
*   **Press-and-Turn:** The encoder button remains digital for 100% reliability during concurrent scrolling actions.

### 1.3 Strategic Farkle Tracking (Status Strip)
The **8-LED NeoPixel Strip** provides a "Danger Map" of the table.
*   **Current Player (Flashing):** White (0), Yellow (1), Red (2+).
*   **Other Players (Solid-Dim):** Off (0), Yellow (1), Red (2+).

---

## 2. The Target State: Hardware v2.5 Pin Map

To accommodate the increased pin cost of the SPI LCD while preserving hardware encapsulation, we employ a "Hybrid Input" model.

| Pin | Component | Function | Status |
| :--- | :--- | :--- | :--- |
| **D0/D1** | **MP3 Player** | **Serial1** (Hardware Audio) | Pending |
| **D2/D3** | **Encoder A/B** | **Interrupts 0/1** (Smooth Scroll) | Pending |
| **D4** | **Encoder SW** | **Digital Input** (SELECT Button) | Pending |
| **D5** | **BANK Button** | **Digital Input** (High Reliability) | **Active (V1)** |
| **D6** | **FARKLE Button** | **Digital Input** (High Reliability) | **Active (V1)** |
| **D7** | **LCD RES** | Hardware Reset for LCD | Pending |
| **D8** | **LCD BLK** | PWM Backlight Control | Pending |
| **D9** | **SD Card CS** | SPI Chip Select (Data) | Pending |
| **D10** | **MAX7219 CS** | SPI Chip Select (Displays) | **Active (V1)** |
| **D11** | **SPI COPI** | Shared Data Out (MOSI) | **Active (V1)** |
| **D12** | **SPI CIPO** | Shared Data In (MISO) | **Active (V1)** |
| **D13** | **SPI SCK** | Shared SPI Clock | **Active (V1)** |
| **A0** | **LED Grid** | NeoPixel Data (8x8) | **Active (V1)** |
| **A1** | **Status Strip** | NeoPixel Data (8-LED) | **DONE (V2)** |
| **A2** | **Scoring Ladder** | **Analog Ladder (+50, +100, +500, CLEAR)** | Pending |
| **A3** | **Latching Switch** | **"Total Score" Toggle** | Pending |
| **A4** | **LCD CS** | SPI Chip Select (LCD) | Pending |
| **A5** | **LCD DC** | Data/Command (LCD) | Pending |

---

## 3. Rationale & Trade-off Analysis

### 3.1 Why the "Hybrid" Resistor Ladder?
*   **Constraint:** The ST7789 IPS display requires 4 dedicated pins (CS, DC, RES, BLK) beyond the SPI bus.
*   **Alternative Considered:** **Full Button Ladder (8 buttons).** Rejected due to "Ground Bounce" noise from the NeoPixel grid causing potential misreads on critical "Game Ender" buttons (BANK/FARKLE).
*   **Alternative Considered:** **Daisy-Chaining LEDs.** Rejected to maintain strict software encapsulation between the `LedProgressGrid` and `FarkleWarningLights` components.
*   **Decision:** Place the 4 "Utility/Scoring" buttons on an Analog Ladder (**A2**). Keep **BANK**, **FARKLE**, and **SELECT** (Encoder) on digital pins for 100% reliability and concurrent "Press-and-Turn" support.

### 3.2 Handling Noise
*   **Challenge:** Large NeoPixel current spikes create high-frequency noise on the analog rail.
*   **Solution:** We implement **Analog Hysteresis** (Dead Zones) in the `ControlPad` library and recommend a **0.1µF capacitor** across the A2-GND pins to smooth the signal.

---

## 4. Migration Steps (The "Safe Point" Approach)

### **Phase 1: Input & Display Foundation**
**Step 1: The Status Strip (A1) [DONE]**

**Step 2: The Hybrid Input Refactor**
*   **Hardware:** Build the 4-button resistor ladder on **A2** (+50, +100, +500, CLEAR). Move **BANK**, **FARKLE**, and **SELECT** to their target digital pins.
*   **Software:** Update `ControlPad` HAL to handle `analogRead` thresholds.
*   **Verification:** Verify 100% button detection accuracy during heavy NeoPixel activity.

**Step 3: The IPS Color Upgrade (ST7789)**
*   **Hardware:** Connect ST7789 to SPI and Control Pins (A4, A5, D7, D8).
*   **Software:** Swap `U8G2` for `Adafruit_ST7789`. Update UI for 240x240 and color-coded text.
*   **Verification:** High-speed, color-accurate UI updates.

### **Phase 2: Navigation & Logic**
**Step 4: Strategic Navigation (Encoder A/B)**
*   **Hardware:** Connect Encoder pulses to D2/D3.
*   **Software:** Implement `encoder.getDelta()` for Competitor Preview and menu scrolling.

**Step 5: The "Total Score" Toggle (A3)**
*   **Software:** Implement the latching switch logic to swap between Banked and Total scores.

### **Phase 3: Persistence & Audio**
**Step 6: SD Card Storage (D9)**
**Step 7: MP3 Audio (D0/D1)**

---

## 5. Rollout Strategy
1.  **Iterative Updates:** `SCHEMATIC_AND_HARDWARE_GUIDE.md` updated after each step.
2.  **Safe Points:** No transition begins until the current hardware is playable and passing tests.