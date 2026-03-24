> **Scope:** Provides a complete wiring guide, pin map, and power strategy for connecting all physical hardware components to the Arduino.
> **Status:** **LIVE DOCUMENT** - This file represents the current source of truth. If hardware changes, this document MUST be updated.

# **The "Retro Arduino" Farkle Scoreboard: Complete Wiring Guide**

This guide covers the wiring for the **Arduino Uno R4 WiFi** to control the 15-digit display, 8x8 NeoPixel grid, LCD, Speaker, and input controls.

**Recommended Board:** Arduino Uno R4 WiFi

**Power Requirement:** External 5V 4A (Minimum) Power Supply.

## **Diagram Language**

The diagram below uses **Mermaid.js**. You can render this using the [Mermaid Live Editor](https://mermaid.live/) or any markdown viewer that supports Mermaid (like GitHub or Obsidian).

## **Power Strategy (CRITICAL)**

**DO NOT** power the NeoPixels or the 7-Segment displays from the Arduino's 5V pin. You will burn out the board.

1. **External 5V PSU (+):** Connects to NeoPixel 5V, MAX7219 VCC, LCD VCC, and the Arduino 5V pin\*.  
2. **External PSU GND (-):** Connects to **ALL** components and the Arduino GND.  
3. **Power Toggle Switch:** Install this on the **positive** line coming from your power brick *before* it splits to the components.

*(Note: Feeding 5V regulated power into the Arduino 5V pin is safe if your power supply is a high-quality, regulated 5V source.)*

### **Capacitor Stabilization Strategy (Issue #80)**

To alleviate display corruption caused by "dirty power" (noise from high-current components like the LED Grid), the following capacitors are required:

1.  **Main Bulk Reservoir**: Install a large **1000µF (10V or 16V) Electrolytic Capacitor** across the main power rails (near where the PSU input enters). This acts as a reservoir to buffer the large current spikes from the NeoPixel grid.
2.  **Local Decoupling**: Install a **0.1µF Ceramic Capacitor** as close as possible to the VCC and GND pins of the **IPS LCD (ST7789)**. This filters out high-frequency noise.
3.  **Input Smoothing (Hybrid Ladder)**: Install a **0.1µF Ceramic Capacitor** between the **A2 (Analog Input)** pin and **GND**. This is crucial for stabilizing the resistor ladder readings during NeoPixel activity.
4.  **Arduino Decoupling**: Similarly, place a **0.1µF Ceramic Capacitor** near the Arduino's 5V and GND pins if using the common power rail.

### **Connecting a USB Power Bank**

Since your power source is a USB-C Power Bank, you need a way to get the power out of the cable and onto your wires.

**The Recommended Method: USB-C Breakout Board**

This is a small PCB (costing \~$2-5) that takes a USB-C cable and breaks out the power lines to easy-to-solder pads.

* **What to buy:** Search for "USB-C Breakout Board VBUS GND" or "USB-C Power Breakout".  
* **Important Feature:** Ensure the board has **5.1kΩ resistors on the CC pins**. This signals the power bank to turn on and provide 5V. If you use a board without these resistors with a USB-C to USB-C cable, the power bank might refuse to send power.  
* **Wiring:**  
  1. Solder a thick **Red Wire** to the pad labeled **VBUS** (or VCC). This goes to your **Toggle Switch**.  
  2. Solder a thick **Black Wire** to the pad labeled **GND**. This goes to your **Common Ground**.

## **Battery & Power Consumption Analysis**

If you plan to run this on batteries, here is the math based on your usage (50% LED active).

### **1\. Estimated Current Draw**

* **NeoPixels (64 Total):**  
  * At 50% usage (32 pixels lit) with mixed colors (avg 40mA/pixel): **\~1.28 Amps**  
* **7-Segment Displays (15 Digits):**  
  * Driven by MAX7219 (Multiplexed): **\~0.30 Amps**  
* **Arduino Uno R4 WiFi \+ LCD \+ Speaker:**  
  * Processor \+ WiFi \+ Sound: **\~0.25 Amps**  
* **TOTAL DRAW:** **\~1.83 Amps** (Round up to **2.0A** for safety)

### **2\. Battery Recommendations**

| Power Source | Capacity | Est. Runtime | Pros/Cons |
| :---- | :---- | :---- | :---- |
| **USB Power Bank** | **10,000mAh** | **\~3.5 Hours** | **Best Choice.** Regulated 5V, safe, rechargeable. ensure it has a **2.4A output** port. |
| **AA Batteries (NiMH)** | **2500mAh (x4)** | **\~1 Hour** | **Okay.** 4x Rechargeable AAs give \~4.8V-5.2V. Voltage will sag quickly under 2A load. |
| **9V Battery** | **500mAh** | **Instant Fail** | **Do Not Use.** Cannot supply 2A; voltage will drop to 0 immediately. |

## **Wiring Diagram**

graph TD

    subgraph Power\_Source \["USB-C Power Bank"\]  
    USB\_OUT\[USB Cable\]  
    end

    subgraph Breakout \["USB-C Breakout Board"\]  
    VBUS\[VBUS Pad\]  
    GND\_PAD\[GND Pad\]  
    end

    subgraph Power\_Distribution  
    SWITCH\[Power Toggle Switch\]  
    GND\_BUS\[Common Ground Bus\]  
    end

    subgraph Arduino \["Arduino Uno R4 WiFi"\]  
    A\_5V\[5V Pin\]  
    A\_GND\[GND Pin\]  
    D2\_D3\["D2/D3 (Encoder A/B)"\]
    D4\["Pin D4 (SELECT)"\]
    D5\["Pin D5 (BANK)"\]
    D6\["Pin D6 (FARKLE)"\]
    D7\["Pin D7 (LCD RES)"\]
    D8\["Pin D8 (LCD BLK)"\]
    D9\["Pin D9 (SD CS)"\]
    D10\["Pin D10 (Score CS)"\]
    D11\["Pin D11 (SPI MOSI)"\]
    D13\["Pin D13 (SPI SCK)"\]
    A0\["Pin A0 (NeoPixel Grid)"\]  
    A1\["Pin A1 (Status Strip)"\]  
    A2\["Pin A2 (Ladder: Scoring/Clear)"\]  
    A3\["Pin A3 (Latching Switch)"\]
    A4\["Pin A4 (LCD CS)"\]
    A5\["Pin A5 (LCD DC)"\]
    end

    subgraph Components  
    NEO\["8x8 NeoPixel Grid"\]  
    MAX\["MAX7219 Score Displays"\]  
    LCD\["ST7789 IPS LCD (SPI)"\]  
    SPK\["MP3 Player + Speaker"\]  
    BTNS\["Hybrid Control Pad"\]  
    STRIP\["8-LED Status Strip"\]  
    SD\["SD Card Module"\]
    end

    %% USB Connection  
    USB\_OUT \--\> Breakout  
    VBUS \--\> SWITCH  
    GND\_PAD \--\> GND\_BUS

    %% Power Distribution  
    SWITCH \--\> A\_5V  
    SWITCH \--\> NEO  
    SWITCH \--\> MAX  
    SWITCH \--\> LCD  
    SWITCH \--\> STRIP  
    SWITCH \--\> SD
    GND\_BUS \--\> A\_GND  
    GND\_BUS \--\> NEO  
    GND\_BUS \--\> MAX  
    GND\_BUS \--\> LCD  
    GND\_BUS \--\> SPK  
    GND\_BUS \--\> BTNS  
    GND\_BUS \--\> STRIP  
    GND\_BUS \--\> SD
    
    %% Shared SPI Bus
    D11 \--\> MAX
    D11 \--\> LCD
    D11 \--\> SD
    D13 \--\> MAX
    D13 \--\> LCD
    D13 \--\> SD

    %% Dedicated Control Signals  
    A0 \--\> NEO  
    D10 \--\> MAX
    A4 \--\> LCD
    A5 \--\> LCD
    D7 \--\> LCD
    D8 \--\> LCD
    D9 \--\> SD
    D4 \--\> BTNS
    D5 \--\> BTNS
    D6 \--\> BTNS
    A2 \--\> BTNS
    A3 \--\> BTNS
    D2\_D3 \--\> BTNS
    A1 \--\> STRIP  

## **Detailed Pin Map**

### **1. Shared SPI Bus (D9 - D13)**
Both the **Score Displays** and the **IPS LCD** share the hardware SPI clock and data lines for maximum efficiency.

| Arduino Pin | Signal | Component | Pin Label |
| :---- | :---- | :---- | :---- |
| **D11** | **MOSI (COPI)** | Shared Bus | **DIN** (Score) / **SDA** (LCD) |
| **D13** | **SCK (Clock)** | Shared Bus | **CLK** (Score) / **SCL** (LCD) |
| **D10** | **CS (Load)** | **Score Display** | **CS** |
| **A4** | **CS** | **ST7789 LCD** | **CS** (or GND if missing) |
| **D9** | **CS** | **SD Card** | **CS** |

### **2. IPS Color LCD (ST7789)**
| Arduino Pin | Signal | Notes |
| :---- | :---- | :---- |
| **A5** | **DC** | Data/Command Toggle |
| **D7** | **RES** | Hardware Reset |
| **D8** | **BLK** | Backlight PWM (Brightness) |

### **3. Hybrid Control Pad**

| Input Type | Pin | Action | Rationale |
| :---- | :---- | :---- | :---- |
| **Digital** | **D2/D3** | **Encoder A/B** | High-speed rotation interrupts. |
| **Digital** | **D4** | **SELECT** | Encoder Push Button (Reliable during rotation). |
| **Digital** | **D5** | **BANK** | Dedicated high-priority action. |
| **Digital** | **D6** | **FARKLE** | Dedicated high-priority action. |
| **Digital (A3)** | **A3** | **Latching SW** | Static state (Total Score toggle). |
| **Analog** | **A2** | **Ladder** | **+50, +100, +500, CLEAR**. |

### **4. LED & Audio**

| Arduino Pin | Component | Function |
| :---- | :---- | :---- |
| **A0** | **8x8 LED Grid** | Visual scoring matrix. |
| **A1** | **Status Strip** | Turn pointers & danger levels. |
| **D0/D1** | **MP3 Module** | Serial1 UART for audio events. |

## **Hybrid Resistor Ladder (A2)**

The ladder uses a pull-up resistor to create unique voltage zones for four buttons.

*   **VCC \-\> 10kΩ Resistor \-\> A2**
*   **A2 \-\> Button \-\> R\_Zone \-\> GND**

| Button | R\_Zone | Approx. ADC (10-bit) |
| :--- | :--- | :--- |
| **CLEAR** | 0Ω (Direct) | 0 (\~0V) |
| **+50** | 1kΩ | \~93 (\~0.45V) |
| **+100** | 4.7kΩ | \~328 (\~1.6V) |
| **+500** | 10kΩ | \~512 (\~2.5V) |
| **NONE** | Open | 1023 (\~5.0V) |


## **Updated Code Concepts for Uno R4**

Since you are using the Uno R4 WiFi, you can take advantage of the ArduinoLEDMatrix library if you were using the built-in grid, but for your external NeoPixels, stick to Adafruit\_NeoPixel.

### Setup Snippet
```cpp
// Score Display (Hardware SPI + D10 CS)
// Pins: MOSI=11, SCK=13, CS=10
const int SCORE_DATA_PIN = 11;
const int SCORE_CLK_PIN = 13;
const int SCORE_CS_PIN = 10;

// Text Display V2 (Hardware SPI + Control Pins)
// Pins: MOSI=11, SCK=13, CS=A4, DC=A5, RES=7, BLK=8
const int LCD_CS_PIN = A4;
const int LCD_DC_PIN = A5;
const int LCD_RES_PIN = 7;
const int LCD_BLK_PIN = 8;

void setup() {  
  // Initialize Digital Buttons
  pinMode(BANK_PIN, INPUT_PULLUP);
  pinMode(FARKLE_PIN, INPUT_PULLUP);
  pinMode(SELECT_PIN, INPUT_PULLUP);

  // Initialize Backlight
  pinMode(LCD_BLK_PIN, OUTPUT);
  digitalWrite(LCD_BLK_PIN, HIGH); // Turn on screen

  // ... Initialize Score Display (LedControl), LCD (Adafruit_ST7789), etc ...  
}
```


void loop() {  
  // Example Logic  
  // Update status strip based on game state
}  
```
