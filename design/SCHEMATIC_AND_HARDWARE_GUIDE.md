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
    D2\_8\["Pins D2-D8 (Buttons)"\]  
    D9\["Pin D9 (Speaker)"\]  
    D10\_12\["Pins D10-D12 (MAX7219)"\]  
    A4\_A5\["SDA/SCL (LCD)"\]  
    A0\["Pin A0 (NeoPixel Data)"\]  
    A1\["Pin A1 (Yellow LED)"\]  
    A2\["Pin A2 (Red LED)"\]  
    end

    subgraph Components  
    NEO\["8x8 NeoPixel Grid"\]  
    MAX\["3x MAX7219 Drivers"\]  
    LCD\["I2C LCD Screen"\]  
    SPK\[Speaker \+ Slide Switch\]  
    BTNS\["7x Push Buttons"\]  
    LEDS\["Farkle Tracker (Yellow/Red)"\]  
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
    SWITCH \--\> LEDS  
    GND\_BUS \--\> A\_GND  
    GND\_BUS \--\> NEO  
    GND\_BUS \--\> MAX  
    GND\_BUS \--\> LCD  
    GND\_BUS \--\> SPK  
    GND\_BUS \--\> BTNS  
    GND\_BUS \--\> LEDS

    %% Signals  
    A0 \--\> NEO  
    D10\_12 \--\> MAX  
    D9 \--\> SPK  
    D2\_8 \--\> BTNS  
    A4\_A5 \--\> LCD  
    A1 \--\> LEDS  
    A2 \--\> LEDS

## **Detailed Pin Map**

### **1\. Displays (MAX7219 Daisy Chain)**

*Refer to the previous section for the specific digit wiring between chips.*

| Arduino Pin | Component Pin | Notes |
| :---- | :---- | :---- |
| **D12** | DIN (Chip 1\) | Data Input |
| **D11** | CLK (Shared) | Clock |
| **D10** | CS/LOAD (Shared) | Chip Select |

### **2\. NeoPixel Grid (8x8)**

*Requires high current. Ensure direct connection to PSU.*

| Arduino Pin | Component Pin | Notes |
| :---- | :---- | :---- |
| **A0** | DIN / Data In | You can treat A0 as a digital pin (Pin 14\) |

### **3\. Farkle Tracker LEDs (New)**

*Standard LEDs. Use a 220Ω or 330Ω resistor in series with each LED to prevent burning them out.*

| Arduino Pin | Color | Logic |
| :---- | :---- | :---- |
| **A1** | **Yellow LED** | 1 Farkle (Warning) |
| **A2** | **Red LED** | 2 Farkles (Danger) |

### **4\. Inputs (7 Buttons)**

*Wire one side of the button to the Pin, the other side to GND. We will use internal pull-up resistors in code (INPUT\_PULLUP), so no external resistors are needed.*

| Arduino Pin | Function | Game Logic Example |
| :---- | :---- | :---- |
| **D2** | Button 1 | Left |
| **D3** | Button 2 | Right |
| **D4** | Button 3 | Up |
| **D5** | Button 4 | Down |
| **D6** | Button 5 | Action A |
| **D7** | Button 6 | Action B |
| **D8** | Button 7 | Start/Pause |

### **5\. Audio**

*The slide switch acts as a hardware "Mute".*

| Arduino Pin | Circuit Path |
| :---- | :---- |
| **D9** | Pin D9 \-\> 220Ω Resistor \-\> Slide Switch \-\> Speaker (+) \-\> Speaker (-) \-\> GND |

### **6\. LCD Screen (I2C)**

*Used for settings, Wi-Fi status, or text readouts.*

| Arduino Pin | LCD Pin | Notes |
| :---- | :---- | :---- |
| **SDA** (or A4) | SDA | Data |
| **SCL** (or A5) | SCL | Clock |

## **Updated Code Concepts for Uno R4**

Since you are using the Uno R4 WiFi, you can take advantage of the ArduinoLEDMatrix library if you were using the built-in grid, but for your external NeoPixels, stick to Adafruit\_NeoPixel.

### **Setup Snippet**
```
// Buttons  
const int btnPins\[\] \= {2, 3, 4, 5, 6, 7, 8};  
// Tracker LEDs  
const int YELLOW\_LED \= A1;  
const int RED\_LED \= A2;

void setup() {  
  // Initialize Buttons  
  for (int i \= 0; i \< 7; i++) {  
    pinMode(btnPins\[i\], INPUT\_PULLUP);  
  }

  // Initialize Tracker LEDs  
  pinMode(YELLOW\_LED, OUTPUT);  
  pinMode(RED\_LED, OUTPUT);  
    
  // ... Initialize WiFi, LCD, etc ...  
}

void loop() {  
  // Example Logic  
  // if (farkleCount \== 1\) { digitalWrite(YELLOW\_LED, HIGH); }  
}  
```
