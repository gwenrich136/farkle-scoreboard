> **Scope:** Defines the hardware wiring, software component design, and sound effect mapping for the MP3-TF-16P audio module integrated into the Farkle Scoreboard.
> **Status:** **LIVE DOCUMENT** - This file represents the current source of truth. If code changes, this document MUST be updated.

# Sound Effects System

This document specifies the complete sound effects subsystem for the Farkle Scoreboard. It covers the physical hardware integration (MP3 module, speaker, and mute switch), the `SoundPlayer` software component, and the mapping of game events to specific audio cues.

---

## 1. Hardware Overview

### 1.1 Components

| Component | Model | Specs |
|:----------|:------|:------|
| **MP3 Module** | MP3-TF-16P (DFPlayer Mini / MOD-157) | 16-pin, UART-controlled, onboard 3W BTL amplifier, micro SD slot |
| **Speaker** | 36mm passive speaker | 0.5W, 8Ω, two solder leads |
| **Mute Switch** | DPDT mini toggle switch | Disconnects both speaker lines simultaneously |
| **Protective Resistor** | 1kΩ through-hole resistor | Inline on UART RX line |

### 1.2 Module Pinout

The MP3-TF-16P has 16 pins. Only 6 are used in this project:

| Pin | Name | Connected To | Notes |
|:---:|:-----|:-------------|:------|
| 1 | **VCC** | 5V Power Rail | 3.2V–5V input |
| 2 | **RX** | Arduino D1 (via 1kΩ resistor) | UART receive from Arduino TX |
| 3 | **TX** | Arduino D0 | UART transmit to Arduino RX |
| 5 | **SPK_1** | DPDT Switch Common 1 | Speaker positive (BTL output) |
| 6 | **GND** | Common Ground Bus | |
| 7 | **SPK_2** | DPDT Switch Common 2 | Speaker negative (BTL output) |

**Unused pins (no connection required):** DAC_R (4), DAC_L (8), IO_1 (9), GND (10), IO_2 (11), ADKEY_1 (12), ADKEY_2 (13), BUSY (14), USB+ (15), USB– (16). These are for standalone player mode, headphone/line-out, and USB file transfer — none of which apply to this project.

### 1.3 Wiring

```
Arduino Uno R4 WiFi              MP3-TF-16P Module
─────────────────────            ─────────────────

    5V (Power Rail) ────────────── Pin 1 (VCC)
    GND (Common)    ────────────── Pin 6 (GND)

    D1 (TX / Serial1 TX) ──[1kΩ]── Pin 2 (RX)
    D0 (RX / Serial1 RX) ────────── Pin 3 (TX)


                                           DPDT Switch
                                          ┌──┐
                     Pin 5 (SPK_1) ───────┤  ├─── Speaker (+)
                                          │  │
                     Pin 7 (SPK_2) ───────┤  ├─── Speaker (–)
                                          └──┘
                                     ON = connected
                                     OFF = both lines open (floating)
```

**Critical notes:**
-   **1kΩ Resistor:** The DFPlayer operates on 3.3V logic internally. The Arduino Uno R4 WiFi outputs 5V on its TX line (D1). The 1kΩ resistor on the RX line protects the module from being over-driven. Solder it inline between D1 and Pin 2.
-   **Serial1:** The Uno R4 WiFi provides a dedicated hardware UART on D0/D1 via `Serial1`, completely independent of the USB `Serial` port. No SoftwareSerial is needed.
-   **Power:** The module draws up to ~200mA during playback. The 0.5W 8Ω speaker is well within the module's 3W amplifier capacity. Power from the existing 5V rail is sufficient; no changes to the power budget are needed.

### 1.4 Mute Switch Design

The DFPlayer's speaker outputs (SPK_1 and SPK_2) use a **Bridge-Tied Load (BTL)** amplifier configuration. Both pins carry active differential signals — neither is ground-referenced. This has a critical safety implication:

> **Do NOT use a single-pole switch on only one speaker wire.** Disconnecting a single BTL output while the amplifier is active can damage the onboard amplifier chip and produces loud pop/click transients.

**Solution:** A **DPDT (Double Pole, Double Throw)** toggle switch disconnects **both** SPK_1 and SPK_2 simultaneously:

-   **Common terminals:** Wired to SPK_1 (Pin 5) and SPK_2 (Pin 7) on the DFPlayer.
-   **Output terminals (one throw):** Wired to the speaker leads.
-   **Opposite throw terminals:** Left **unconnected** (open). Do not short or ground them.

When the switch is in the "OFF" position, both amplifier outputs float harmlessly with no load. The software is unaware of the mute state and continues sending `play()` commands normally — the sound is simply inaudible.

### 1.5 SD Card (DFPlayer)

The DFPlayer module has its **own micro SD card slot**, completely separate from the Arduino's SD card module used by the `MemoryCard` component. These are two independent file systems with zero interaction.

-   **DFPlayer's SD card:** Holds MP3 audio files only. Read directly by the DFPlayer hardware.
-   **Arduino's SD card (SPI, D9 CS):** Holds game data (`/sys/`, `/partial/`, `/archive/`). Read by the `MemoryCard` component.

**SD Card Requirements:**
-   Format: **FAT16 or FAT32**
-   Maximum size: **32GB**
-   File structure: All MP3 files in a root-level `/mp3/` folder, named with 4-digit prefixes (e.g., `0001.mp3`).

---

## 2. Sound Effect Catalog

### 2.1 File Mapping

Each `SoundEffect` enum value maps to a specific 4-digit file number on the DFPlayer's SD card:

| File | Enum Constant | Description | Duration | Type |
|:-----|:-------------|:------------|:---------|:-----|
| `0001.mp3` | `SFX_SCORE_LOW` | Score click for +50 (base pitch) | ~100ms | One-shot |
| `0002.mp3` | `SFX_SCORE_MID` | Score click for +100 (+1 octave) | ~100ms | One-shot |
| `0003.mp3` | `SFX_SCORE_HIGH` | Score click for +500 (+2 octaves) | ~100ms | One-shot |
| `0004.mp3` | `SFX_BANKING` | Cha-ching / coin cascade (sustaining) | 10–15s | Sustain + stop |
| `0005.mp3` | `SFX_FARKLE` | Buzzer / sad trombone (sustaining) | 10–15s | Sustain + stop |
| `0006.mp3` | `SFX_PENALTY_FARKLE` | Alarm siren / klaxon | ≥5s | Timed stop |
| `0007.mp3` | `SFX_FINAL_ROUND_BELL` | Track & field bell (single ring) | ~2s | One-shot |
| `0008.mp3` | `SFX_VICTORY_1` | Victory fanfare variant 1 | 5–15s | One-shot |
| `0009.mp3` | `SFX_VICTORY_2` | Victory fanfare variant 2 | 5–15s | One-shot |
| `0010.mp3` | `SFX_VICTORY_3` | Victory fanfare variant 3 | 5–15s | One-shot |

**Score Clicks:** The three score click files are the same source sample pitched at three octaves. This creates an intuitive audio hierarchy where higher-value buttons produce higher-pitched sounds. Create these using a single click sample and applying pitch shifts of +12 semitones (one octave) and +24 semitones (two octaves).

**Victory Fanfares:** The system supports an arbitrary number of victory songs. To add more, place additional files (`0011.mp3`, `0012.mp3`, ...) on the SD card and increment `SFX_VICTORY_COUNT`. The `playRandomVictory()` method selects one at random each time a game is won.

### 2.2 Playback Behaviors

There are three distinct playback patterns used across the game:

1.  **One-shot:** The sound is triggered once and plays to completion. No `stop()` call is needed. Used for: score clicks, final round bell, victory fanfares.

2.  **Sustain + stop:** A long MP3 file is started at the beginning of an animation. The file is long enough to outlast any realistic animation duration (~10-15 seconds). When the animation completes (i.e., `atRiskScore` reaches 0), `stop()` is called to end playback immediately. Used for: banking cha-ching, farkle buzzer.

3.  **Timed stop:** A long MP3 file plays for a specific duration dictated by the animation stage, then `stop()` is called at the stage boundary. Used for: penalty siren (plays through the full 5-second "Pain" stage, stopped when Stage 2 begins).

---

## 3. Game Event → Sound Mapping

### 3.1 Pre-Game Phases

No sound effects are played during pre-game phases. The boot sequence is silent.

### 3.2 In-Game Phases

| Game Event | Phase | Trigger Point | Sound Effect | Playback Pattern |
|:-----------|:------|:-------------|:-------------|:-----------------|
| +50 scored | `WaitingPhase` | On `PLUS_50` input | `SFX_SCORE_LOW` | One-shot |
| +100 scored | `WaitingPhase` | On `PLUS_100` input | `SFX_SCORE_MID` | One-shot |
| +500 scored | `WaitingPhase` | On `PLUS_500` input | `SFX_SCORE_HIGH` | One-shot |
| Player banks points | `BankingPhase` | `onEnter()` | `SFX_BANKING` | Sustain until `atRiskScore == 0`, then `stop()` |
| Player farkles | `FarklingPhase` | `onEnter()` | `SFX_FARKLE` | Sustain until `atRiskScore == 0`, then `stop()` |
| 3rd consecutive farkle | `PenaltyFarklingPhase` | `onEnter()` | `SFX_PENALTY_FARKLE` | Plays through entire 5s "Pain" stage (Stage 1). `stop()` at transition to Stage 2 |
| Final round triggered | `EndOfTurnPhase` | When `state.finalRoundTriggered` is set to `true` | `SFX_FINAL_ROUND_BELL` | One-shot |

### 3.3 Post-Game Phases

| Game Event | Phase | Trigger Point | Sound Effect | Playback Pattern |
|:-----------|:------|:-------------|:-------------|:-----------------|
| Winner declared | `PostGamePhase_V1` | `onEnter()` | `playRandomVictory()` | One-shot (random selection) |

---

## 4. Software Component: `SoundPlayer`

### Purpose
The `SoundPlayer` component provides a fire-and-forget interface for triggering sound effects from game phases. It wraps the `DFRobotDFPlayerMini` library and communicates with the MP3-TF-16P module over `Serial1` (UART, 9600 baud).

### API Design
-   **`SoundPlayer()`**: Constructor. No pin arguments — the component uses `Serial1` (D0/D1) exclusively.
-   **`void begin()`**: Initializes the UART connection (`Serial1.begin(9600)`) and the DFPlayer library. Sets volume to a hardcoded level (e.g., 20 out of 30).
-   **`void play(SoundEffect sfx)`**: Starts playback of the MP3 file corresponding to the given `SoundEffect` enum value. Internally calls `dfPlayer.playFromMP3Folder(sfx + 1)` (enum is 0-indexed; file numbers are 1-indexed).
-   **`void playRandomVictory()`**: Randomly selects one of the victory fanfare variants and plays it. Uses `random(SFX_VICTORY_COUNT)` to pick an index, then calls `play()` with the corresponding enum value.
-   **`void stop()`**: Immediately stops any currently playing sound. Internally calls `dfPlayer.stop()`.

### Key Logic & Behavior
-   **Fire-and-Forget:** Phases call `play()` or `stop()` without needing to poll or manage playback state. The DFPlayer handles decoding and amplification autonomously.
-   **No Mute Awareness:** The software has no knowledge of the hardware mute switch state. Commands are sent regardless; silence is achieved purely by the DPDT switch disconnecting the speaker.
-   **Volume:** Hardcoded via `dfPlayer.volume(20)` during `begin()`. The DFPlayer supports volume levels 0–30.
-   **Library Dependency:** Uses the `DFRobotDFPlayerMini` library (installed via PlatformIO `lib_deps`).
-   **Startup Delay:** The DFPlayer module requires approximately 200ms after power-on before it can accept commands. The `begin()` method should account for this with a brief delay or retry logic.
-   **File Numbering Convention:** The `SoundEffect` enum is 0-indexed for clean C++ usage. File numbers on the SD card are 1-indexed per DFPlayer convention. The `play()` method handles the `+1` offset internally.

### Integration

-   **Ownership:** The `SoundPlayer` is owned by the `Game` class, alongside all other hardware components.
-   **Phase Access:** Phases access the `SoundPlayer` through the `Displays` struct (kept as a legacy naming quirk for consistency with the existing codebase).

```cpp
// Defined in: src/farkle/include/Displays.h
struct Displays {
    ScoreDisplay& scoreDisplay;
    LedProgressGrid& grid;
    FarkleWarningLights& farkleLights;
    TextDisplayV2& textDisplay;
    SoundPlayer& soundPlayer;
};
```

### Defined In
-   **Header:** `src/farkle/lib/components/SoundPlayer/SoundPlayer.h`
-   **Implementation:** `src/farkle/lib/components/SoundPlayer/SoundPlayer.cpp`

---

## 5. Testing Strategy

### Tier 1: Game Logic (Native)
-   **Mock:** `SoundPlayer` is mocked as a no-op class (see `src/farkle/test/mocks/`).
-   **Scope:** Verifies that the correct `SoundEffect` is triggered at the correct phase transition. For example: entering `BankingPhase` calls `play(SFX_BANKING)`, and reaching `atRiskScore == 0` calls `stop()`.

### Tier 2: Component Logic
-   **Mock:** The `DFRobotDFPlayerMini` library is mocked via a fake header.
-   **Scope:** Verifies that `play(SFX_BANKING)` translates to the correct serial command (`dfPlayer.playFromMP3Folder(4)`), and that `playRandomVictory()` selects a valid file number within the victory range.

### Hardware Verification
-   Manual testing: load the SD card with MP3 files, power up, and play through a complete game listening for correct sound triggers and timing.

---

## 6. Cross-References

The following design documents are impacted by this feature and must be updated when implementation begins:

-   **[COMPONENT_LIBRARIES.md](COMPONENT_LIBRARIES.md):** Add Section 8: `SoundPlayer` with full API and behavior documentation.
-   **[SCHEMATIC_AND_HARDWARE_GUIDE.md](SCHEMATIC_AND_HARDWARE_GUIDE.md):** Add DPDT mute switch to wiring diagram. Confirm D0/D1 pin map entry for MP3 module. Add 1kΩ resistor to soldering notes.
-   **[IN_GAME_PHASES.md](IN_GAME_PHASES.md):** Add sound trigger documentation to each phase's "Implementation Details" section.
-   **[POST_GAME_PHASES.md](POST_GAME_PHASES.md):** Add `playRandomVictory()` call to `PostGamePhase_V1` implementation details.
-   **[GENERAL_GAME_PLAY.md](GENERAL_GAME_PLAY.md):** Update Section 3.1 to replace "(future implementation)" with a reference to this document for the final round bell. Add sound descriptions to banking, farkling, and penalty animation flows.
-   **[TECHNICAL_DESIGN_GAME_STATE_MACHINE.md](TECHNICAL_DESIGN_GAME_STATE_MACHINE.md):** Add `SoundPlayer` to the `Displays` struct definition and `Game` class member list.
-   **[TESTING_STRATEGY.md](TESTING_STRATEGY.md):** Add `SoundPlayer` mock and test case entries for both Tier 1 and Tier 2.
