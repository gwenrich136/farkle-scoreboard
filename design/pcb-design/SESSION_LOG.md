> **Scope:** Compact record of each PCB/enclosure design session (human + AI): who took part, what was read, what was proposed or decided, and what is still open. Follows the project's "vibe coding" practice of distilling every design conversation into markdown in `design/`.
> **Status:** **LIVE LOG** - Append one entry per session, newest last. Decisions recorded here must also be reflected in `MODULAR_PCB_PROPOSAL.md` (or its successor spec).

# PCB & Enclosure Design: Session Log

## Folder Index (`design/pcb-design/`)
| File | Role |
|:--|:--|
| `ELECTRONICS_CURRENT_STATE.md` | Snapshot audit of the hand-wired build: subsystems, pin/power budgets, findings F1–F11 |
| `MODULAR_PCB_PROPOSAL.md` | Proposed board set, interconnect contract, power, certification, enclosure, phasing, open questions Q1–Q9 |
| `VIBE_CODING_PCB_PITFALLS.md` | Recurring mistakes in AI-drafted PCB work, from Bennett's earlier boards; A0→A1 and A1→order checklists |
| `SESSION_LOG.md` | This file |

---

## Session 1: 2026-10-16, Kickoff & Architecture Proposal

**Participants:** Bennett Dixon (helping on schematic/enclosure/PCB), Claude (AI, acting as senior EE reviewer). Guillermo not present, so everything below is a **proposal pending his review**.

**Brief from Bennett:**
1.  Follow this repo's design-doc layout and record sessions as markdown.
2.  All changes via PR (from a fork) to `dev`.
3.  Summarize the current electronics by subsystem: compute, screen, gamepad, power, and so on.
4.  Use **certified PSUs and pre-certified wireless** rather than certifying our own.
5.  Propose a set of PCBs. One board is acceptable, but modular is preferred.

**Read:** every file in `design/` (focus on `SCHEMATIC_AND_HARDWARE_GUIDE.md`, `HARDWARE_UPGRADE_MIGRATION_PLAN.md`, `SOUND_EFFECTS.md`, `COMPONENT_LIBRARIES.md`, `wiring.png`), plus `platformio.ini` and the component pin `#define`s.

**Key findings (see `ELECTRONICS_CURRENT_STATE.md` §5):**
*   **F1:** the no-CS ST7789 shares SPI with the MAX7219s and SD, so it sees their traffic. This is a probable contributor to LCD corruption.
*   **F2:** worst-case load is ~5.8 A against a 3 A USB-C source, with no current limiting.
*   **F3:** LED return current shares the logic ground. This is the likely root of the A2/LCD noise (Issue #80).
*   **F4/F5:** the multi-bit buttons need steering diodes, and two-button chords alias to score buttons (FARKLE+CLEAR → +500).
*   Documentation drift: `wiring.png` is V1; there's a Mermaid fence issue; and AGENTS.md says `esp32dev` while platformio only has `uno_r4_wifi`.

**Proposed (not yet decided):**
*   Four custom designs: **PWR, CORE-A (Uno R4 shield), SEG ×3, PAD**. Buy the grid, strip, LCD (with CS), and DFPlayer.
*   Phase A keeps the Uno R4 WiFi and the **existing pin map**, so there's no firmware change. Optional Phase B moves to a module-down ESP32-S3-WROOM-1 behind the same connectors.
*   Certification boundary: external UL/CE USB-C adapter, no internal battery, radio only via pre-certified module. The product still needs FCC Part 15B and, in the EU, RED.
*   eFuse current limit ~2.9 A plus a firmware LED budget ≤2.4 A.
*   Single-board alternative documented and deferred to "Phase C" once the layout freezes.

**Open questions for Guillermo:** Q1–Q9 in `MODULAR_PCB_PROPOSAL.md` §11. The most blocking are Q1 (goal/certification), Q3 (integrated vs tethered gamepad), and Q4 (current diode/CS reality).

**Next session agenda:**
1.  Walk Guillermo through the findings and answer Q1–Q9.
2.  Freeze the §5 interconnect pinout.
3.  Start KiCad schematics for PWR and CORE-A (create `hardware/kicad/`).

---

## Session 2: 2026-10-16, Build Photo Review

**Participants:** Bennett Dixon, Claude. Input relayed from Guillermo: a photo of the working breadboard build, and the answer to Q1.

**Answered:**
*   **Q1:** personal build, not a sales project. Certification becomes a *design-for* goal rather than a gate (proposal §7). We keep the certified-adapter and pre-certified-radio rules because they're cheap and keep a sale possible later.

**Learned from the photo (audit §7):**
*   The score displays are **off-the-shelf chainable 5-digit MAX7219 modules**, so a custom SEG board is no longer needed for Phase A.
*   Power is a **Xiaomi USB-A power bank** (typically rated 5.1 V/2.4 A). That is lower than the 3 A assumed in Session 1.
*   Power and ground go through breadboard rails and Dupont jumpers (new finding **F12**), which reinforces F3.
*   No mute switch, LED grid, or status strip is visible. F1 (LCD CS) and F4 (button diodes) can't be confirmed from the photo.

**Changed in `MODULAR_PCB_PROPOSAL.md`:**
*   Custom boards drop from four designs to **three: PWR, CORE-A, PAD**. SEG reuses the existing modules, and a custom SEG is optional.
*   eFuse limit goes to **~2.5 A** for the power bank (2.9 A with a 3 A USB-C adapter). The firmware LED budget goes to **≤2.0 A**.
*   Note added that CC-based source sensing can't work with USB-A banks.
*   PAD notes the current KY-040 encoder and self-locking switch modules.

**Still open:** Q2–Q9, especially Q3 (integrated vs tethered pad) and Q4 (LCD CS pin count, button diodes). A close-up photo of the LCD header and the underside of the button perfboard would answer Q4.

---

## Session 3: 2026-10-16, Portable Power

**Participants:** Bennett Dixon, Claude.

**Context:** Guillermo likely isn't attached to the current power bank, but portability is probably a feature he wants. Bennett asked whether a pre-certified battery module could be specced in.

**Proposed (`MODULAR_PCB_PROPOSAL.md` §6.1):**
*   **Recommended:** a certified USB-C PD power bank (5 V/3 A, ≥10,000 mAh) **docked** in an enclosure bay and removable. It carries all battery certification itself, and we design no charger or cell circuitry.
*   **Upgrade (PWR-2):** a second "wall" USB-C inlet with a priority power mux, so plugging in takes over without relying on the bank's pass-through charging. Many banks briefly drop their output during pass-through, which would reset the MCU.
*   **Rejected:** certified cells with our own charger and boost (brings in battery-safety design), and bare 18650s or uncertified power-bank boards.
*   Runtime is ~4 h typical on 10,000 mAh and ~8 h on 20,000 mAh.

**Decided:** the build photo is **not** committed to the repo.

**New open question:** Q10 (portability, and A vs B).

---

## Session 4: 2026-10-16, Design Review R1 and the Pitfalls Guide

**Participants:** Bennett Dixon, Claude (as senior EE reviewer). Reviewed `MODULAR_PCB_PROPOSAL.md` as it stood after Session 3, against the RA4M1, 74AHCT/74LVC, TPS2595 and DFPlayer datasheets from memory (every number below is to be re-confirmed from the PDF at A0, per the pitfalls guide §2).

**Checked and left alone:** the LCD level shift (74LVC245A at 3.3 V has 5 V-tolerant inputs); the SD MISO up-shift (74AHCT1G125 at 5 V reads a 3.3 V high, its active-low OE matches the active-low CS on D9); the LCD clock-gate fallback; MAX7221 as a register-compatible MAX7219 drop-in with `ScoreDisplay`'s CS-low/shift/CS-high sequence; the 6-pin power-only USB-C receptacle (VBUS×2, GND×2, CC1, CC2) with separate Rd; J6's reserved VPAD pin and the Phase B I2C reuse; the 3V3 LDO sizing for LCD + microSD; the power-bank options and runtime arithmetic.

**Findings, all applied to the proposal:**

| # | Sev | Finding | Fix |
|:--|:--|:--|:--|
| R1 | M | Steering diode count was 10. Single-bit buttons need none (their only shared node is GND); only the multi-bit buttons need one per line. | **7 diodes.** The mechanism is now explained in §4.4. |
| R2 | M | The 5 V buffer needed 5 channels (MOSI, SCK, CS, A0, A1); a 74AHCT125 has 4. | One **74AHCT244** (8 ch). |
| R3 | M | The J6 RC network's order was unspecified. With the pull-up on the MCU side of the series 1 kΩ, a pressed line sits at ≈0.73 V against the RA4M1's V_IL of 1.0 V. | Pull-up at the connector node, series R toward the MCU, cap at the MCU side. |
| R4 | M | eFuse limit (2.5 A) sat *above* the 2.4 A bank's rating, so the bank's protection would trip first. | Limit ≈ source rating − 10 %: 2.2 A (bank), 2.7 A (3 A source). |
| R5 | L | Inrush into ~1.3 mF wasn't tied to the soft-start setting. | dV/dt capacitor for a ≥ 6 ms rise (< 1 A). |
| R6 | M | Current limiting doesn't stop a mid-write power loss corrupting the SD journal. | Firmware note: CRC per journal record, drop a torn last record; eFuse fault pin in Phase B. |
| R7 | M | The BOM was listed as a repo addition "once agreed", i.e. after layout. The part sets the footprint. | New **Phase A0** (BOM from the fab's catalog before schematic capture), principle 7, `bom.csv` beside each KiCad project, a review doc per revision. |
| R8 | L | A4/A5 double as `Wire` SDA/SCL on the R4; F8 had no concrete test. | Firmware guard noted; DMM diode-mode check for the USB/5 V path. |
| R9 | L | J8's EN line had no defined polarity, pull or protection. | Pull-down, switch to a pull-up, RC + TVS; a shorted cable turns the unit *on* (harmless). |
| R10 | M | DFPlayer TX (3.3 V) into the RA4M1 RX is below the CMOS V_IH of 4.0 V, and the bare 1 kΩ on the TX line relies on the module's clamp. | TX through a '244 channel; a 1k/2k divider on the RX line. |

**New document:** `VIBE_CODING_PCB_PITFALLS.md`, 13 pitfalls from Bennett's sensor/control boards (v1.0 → v1.4: pitch change, USB-C change, hole relocation after layout) and the 24 V power board (v0 → v0.6, three reviews in two days; fab export with no part numbers; a reset that rebooted the host, which the bring-up test would have passed; a generator that orphaned every routed footprint). Its two checklists gate A0 → A1 and A1 → order in the proposal's §9.

**Still open:** Q2–Q10. **Q8 (fab) now gates A0**, because the BOM is chosen from that fab's catalog.

---

## Session 5: 2026-11-01, Guillermo Q&A & Single-Board Architecture Lock-In

**Participants:** Guillermo, Antigravity (AI).

**Context:** Walked through the schematic audit of `FarkleScorekeeper.kicad_sch` and resolved all remaining open questions (`Q2`–`Q10`) from `MODULAR_PCB_PROPOSAL.md` §11.

**Answered (`Q1`–`Q10` all closed):**
*   **Q1 (Goal):** Personal build (confirmed previously).
*   **Q2 & Q5 (Compute & WiFi):** Keep the **Arduino Uno R4 WiFi** plugged into headers on the back of the PCB (Phase A only; zero firmware changes).
*   **Q3 & Board Architecture:** **Single Unified Main PCB** mounted behind the front panel (instead of the 3-board modular split). Controls, 72 SMD LEDs, and displays sit on the front side; Uno R4 WiFi, 5V microSD adapter (`MC_A1`), and DFPlayer (`MP3-TF-16P`) plug into headers on the back; power switch and USB-C breakout/docked power bank connect via a 2-pin power entry header.
*   **Q4 (LCD CS & Button Diodes):**
    *   **F1 resolved:** Guillermo upgraded to an **8-pin ST7789 240x240 LCD module that includes a `CS` pin** (`TextDisplay` / `TD1` in the KiCad schematic). The `74LVC1G125` clock-gate fallback is no longer needed.
    *   **F4 resolved:** The schematic confirms **7 steering diodes** (`D1`–`D7`) on the multi-bit buttons (`+50`, `+100`, `+500`).
*   **Q6 (SD Access):** **Internal-only** (set and forget inside the enclosure; reuses the 5V-native `MemoryCard_Adapter` module `MC_A1`, so discrete SD level-shifter ICs are not needed).
*   **Q7 (Displays, LEDs, Controls):**
    *   **Score Displays:** Reuse the **3× off-the-shelf 5-digit MAX7219 modules** (`AtRiskScore`, `Banked`, `Opposition` adapters on the PCB).
    *   **LED Grid & Strip:** Place **72 individual SMD `WS2812B` LEDs** (64 for the 8x8 progress grid + 8 for the status strip, with `100nF` decoupling per LED) directly on the PCB, organized in a clean **KiCad hierarchical sub-sheet**.
    *   **Buttons & Controls:** PCB-mounted tactile switches (`BANK`, `FARKLE`, `CLEAR`, `+50`, `+100`, `+500`), PCB-mounted rotary encoder, and PCB-mounted DPDT latching switch (`SW2`).
*   **Q8 (Fab & Assembly):** **JLCPCB / PCBWay SMD assembly** for the 72 SMD LEDs, passives, diodes, and active protection/level-shifter ICs; hand-solder through-hole headers and switches.
*   **Q9 (Chord Aliasing):** **Accept as-is** (rely on physical button spacing + 50 ms debounce window).
*   **Q10 (Portability & Power):** **Option A** — single 5V USB-C power entry (docked USB-C power bank or wall adapter) routed through the panel power rocker switch.

**Circuit Protection & Level-Shifting Decision:**
*   Include the 3 active protection/level-shifting upgrades on the Single Unified PCB (assembled via SMD pick-and-place):
    1.  **Power Entry eFuse (`TPS2595`):** Soft-start for the `1000µF` bulk capacitor, ~2.2A–2.5A over-current limit, and low-current `EN` switch input (plus `SMBJ5.0A` TVS diode).
    2.  **LCD 3.3V Level Shifter (`AP2112K-3.3` LDO + `74LVC245A`):** Steps 5V SPI/control signals (`SCL/SCK`, `SDA/MOSI`, `RES`, `DC`, `CS`, `BLK`) down to 3.3V for the ST7789 LCD (`TD1`).
    3.  **5V Buffer (`74AHCT244`) + DFPlayer `1kΩ/2kΩ` RX Divider:** Shifts DFPlayer 3.3V `TX` up to 5.0V for Uno R4 `D0 (RX)` (satisfying the RA4M1 $4.0\text{V}$ CMOS $V_{IH}$) and buffers the `A0`/`A1` NeoPixel and `MAX7219` SPI lines.

**Schematic Audit Findings Fixed in Repo KiCad Project (`hardware/kicad/FarkleScorekeeper/`):**
*   Duplicated `FarkleScorekeeper` KiCad project into `hardware/kicad/FarkleScorekeeper/` (keeping the original external project untouched).
*   Fixed `AtRiskScore` `CS_IN` (moved from `A4` to `D10`; `A4` connected to `TextDisplay` `CS`).
*   Connected broken daisy-chain wires (`D_OUT`/`CS_OUT`/`CLK_OUT` -> `D_IN`/`CS_IN`/`CLK_IN`) between `Banked` and `Opposition` MAX7219 adapters.
*   Swapped the reversed diode buses on `+50` (needs 3 diodes: `D5`, `D6`, `A2` for binary `111`) and `+100` (needs 2 diodes: `D5`, `D6` for binary `110`).
*   Unified all floating `+5V` and `GND` islands with proper power symbols and decoupling capacitors.
*   Added the missing subsystems: `TextDisplay` (`TD1`) level-shifted wiring, `DFPlayer Mini` (`MP3-TF-16P`) with `1kΩ/2kΩ` RX divider + `74AHCT244` TX buffer + ferrite-filtered supply, DPDT mute switch (`SW_MUTE`) + speaker header, `TPS2595` eFuse power entry + power switch header, and the 72-LED `WS2812B` hierarchical sub-sheet (`LedGrid_StatusStrip.kicad_sch`).
*   **ERC Verification:** Ran `kicad-cli sch erc` across `FarkleScorekeeper.kicad_sch` and `LedGrid_StatusStrip.kicad_sch`: **0 Errors, 0 Warnings**.
*   **Full Component & Net Reference:** Added to `MODULAR_PCB_PROPOSAL.md` §10.1.

**Next Steps (Phase A0 → A1):**
1.  Select physical mechanical switches (`BANK`/`FARKLE`/`CLEAR`/`+50`/`+100`/`+500`, `SW2`, `SW_MUTE`, `RotaryEncoder`) and JLCPCB/LCSC part numbers for the SMD BOM.
2.  Assign footprints to all schematic symbols in `FarkleScorekeeper_v2` and proceed to PCB layout.



