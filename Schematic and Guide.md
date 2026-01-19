# **The "Retro Arduino" Game Console: Complete Wiring Guide**

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

## **Wiring Diagram![][image1]**

### **Diagram Code**

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
    MAX\["2x MAX7219 Drivers"\]  
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


[image1]: <data:image/png;base64,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>