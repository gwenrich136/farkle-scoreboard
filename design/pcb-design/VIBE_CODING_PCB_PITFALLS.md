> **Scope:** The mistakes that recur when a PCB is designed the way this repo designs software: an AI drafts, a human reviews, routes and orders. Drawn from Bennett's two previous board families (a sensor/control board set that went v1.0 → v1.4 over several re-spins, and a 24 V power board that went v0 → v0.6 through three reviews in two days). Checklists at the end gate the phases in `MODULAR_PCB_PROPOSAL.md` §9.
> **Status:** **LIVE GUIDE** - Add a pitfall when one bites. Each entry says what happened, why, and the rule that prevents it.

# Vibe-Coding PCBs: Common Pitfalls

An AI can write a schematic netlist, a parts list and a placement, and it can review all three. It cannot see a footprint, open a supplier page, or feel a connector seat. Every pitfall below is one of those blind spots.

---

## 1. Set the BOM from the fab's catalog *before* layout

**The big one.** The part sets the footprint, the footprint sets the layout, and the layout sets the enclosure. Choose parts last and all three move.

**What happened:**
*   The power board was routed on four layers and exported in the fab's format with **every BOM line still without a part number**. Each line was a value ("10 µF 50 V X7R") and a package the AI assumed. At LCSC, 10 µF 50 V X7R is a **1210**, not the 0805 that reads naturally; a line like that changes footprint at order time, after the routing is done.
*   On the sensor boards, the terminal pitch changed to 5 mm *after* a full layout ("convert to 5mm pitch headers"), and the USB-C receptacle changed from through-hole to SMD a day later. Each was a re-layout.

**The rule:** before the schematic is drawn in CAD:
1.  Pick the fab (JLCPCB or PCBWay; this guide says JLCPCB where it matters).
2.  For **every** BOM line, choose a concrete part in the fab's parts library and record its **LCSC code, manufacturer part number, and package**. Prefer **Basic / Preferred** parts, which carry no per-reel loading fee; an Extended part costs a fee per *part type*, so consolidate values (one 10 kΩ, one 100 nF, one Schottky).
3.  Check **stock ≥ 10× the quantity needed**. A part with 40 in stock is not a part.
4.  Build the footprint library **from those parts** (the fab's footprint or the KiCad library footprint matched to the datasheet drawing), then draw the schematic with those footprints already assigned.
5.  Keep the list as a file beside the KiCad project (`<board>/bom.csv` with columns Ref, Value, Package, MPN, LCSC, Basic/Extended, Stock-at-selection, Datasheet URL). The export tool reads this file; a line without an LCSC code is an error, not a warning.

**A BOM line is done when it looks like the second row, not the first:**

| State | Example |
|:--|:--|
| Not done | `C2, 1000 µF 10 V electrolytic` |
| Done | `C2, 1000 µF 10 V, CP_Elec_10x10.5 (radial SMD), Lelon VE-102M1ATR-1010, LCSC C134817, Extended, stock 11 400, <datasheet URL>` |

*(The example LCSC code and stock number above are placeholders; the real ones come from the catalog page at selection time, never from memory.)*

## 2. The AI makes up part numbers

**What happened:** the first power-board BOM carried `MF-SM110/33`, a polyfuse that does not exist. The nearest real part is `MF-SM100/33-2`. It was caught at the first review; a lazier review would have found it at order time.

**The rule:** every manufacturer part number is opened on the supplier page before it enters a doc. The AI proposes; the catalog confirms. The same goes for **pin maps** and **absolute maximums** quoted from memory: open the datasheet PDF, find the table, copy the row.

## 3. Footprint pin mapping is where the real bugs hide

**Why:** the symbol is right, the footprint is right, and the **mapping** between them is wrong. The AI never sees either.

| Part class | What to check |
|:--|:--|
| SOT-23 transistors / MOSFETs | Pin order differs between manufacturers for the same package (G-D-S vs G-S-D). Check the datasheet's top view. |
| SMA/SMB/SOD diodes, TVS | Cathode band vs footprint pin 1. KiCad's `D_SMA` puts the cathode on pin 1; check the symbol agrees. |
| LEDs | Anode/cathode vs the pad marked on the footprint. Reel orientation is not polarity. |
| Connectors | Pin 1 location, **top-entry vs side-entry** (JST `B*B-*H` vs `S*B-*H`), and the mating cable's pin 1. |
| Regulators / eFuses | Exposed pad is a pin; is it GND or floating? Thermal vias under it. |
| Modules on headers | The header's pin order read off the module's **silkscreen**, not a vendor listing. |

**The rule:** before layout, one pass through every footprint with the datasheet drawing open. Record "checked, left alone" items; a review that only lists findings has not shown what it covered.

## 4. Pick-and-place rotation

**What happened:** the fab's placement preview turns SOT and SOIC parts differently from KiCad's rotation convention.

**The rule:** always open the fab's **assembly preview** before paying, and check every polarized or asymmetric part (diodes, ICs, electrolytics, connectors). Three fiducials and the placement origin at a board corner make that preview trustworthy.

## 5. Keep schematic ↔ PCB identities stable

**What happened:** a generator rewrote the schematic after the board had been routed. KiCad's *Update PCB from Schematic* then wanted to **replace all 130 footprints**, because every symbol had a new identity. The routed copper would have been orphaned.

**The rule:** once routing starts, the schematic is edited in place, never regenerated. If a tool writes the schematic, its symbol identities derive from the board name and the reference, not from a fresh UUID per run. The fab export refuses a board whose footprints don't match the schematic.

## 6. Review every revision, and write the review down

**What happened:** the power board went v0.2 → v0.6 in two days through three review passes. The third found that **every reset would reboot the host computer** by charge sharing across a MOSFET. The planned bring-up test ("switch on, read 24 V with a meter") would have passed while the Pi rebooted next to it.

**The rule:** a review pass per revision, written as a doc with **"what was checked and left alone"** as well as findings, with the reasoning kept (why a Miller capacitor was rejected matters more than that it was). A finding's fix gets its own line in the doc's Revisions list.

## 7. Ask about the transients, because the AI won't

A netlist is a DC picture. Ask, every time:
*   **Plug-in:** what charges, through what, and can the gate of a high-side switch lag its source and turn on by itself?
*   **Switch-on:** what capacitance is charged through the switch? Where should the reservoir sit (upstream of the switch, with little downstream)? Does a regulator downstream drop out during the dip?
*   **Switch-off / brown-out:** what is being written to flash or SD at that moment?
*   **A short on each connector pin to each neighbour:** does any pair turn something on, bypass a safety, or put a rail on a ground pin? Choose the **pin order** so no adjacent short does.

## 8. Harness and cable pitfalls

*   **Pre-crimped JST leads are often reversed.** A reversed lead put 5 V on a Pi's ground pins. Make critical leads by hand, and **meter every lead** before it's plugged in.
*   Connectors carry **numbered references for the fab** (J1 …) and **the doc's name as the value**, printed beside each header on the silkscreen. Otherwise the doc says `J_LCD` and the board says `J3`.
*   Keep a **wiring sheet** per cable: pin, net, colour, length, twisted pair or not.

## 9. Layers and heat

**What happened:** a two-layer attempt at a 3 A, three-regulator board left connections open with power traces threaded between capacitors. Four layers (ground plane, power regions, signals) routed cleanly and cost a few dollars more. A 3 A buck on two layers ran about 70 °C above ambient by the datasheet's thermal figure.

**The rule:** anything over ~1 A per rail or with a switching regulator goes to four layers at hobby sizes. Pours and thermal vias under exposed pads. Ask for 2 oz copper if offered. Derate polyfuses and electrolytics for the enclosure's temperature, not the bench's.

## 10. Mechanical before ordering

*   **Component heights vs the enclosure:** the D-sub's flange offset, the electrolytic cans vs the lid, the module on headers vs the window. Measure the real part; the AI's "typical" dimension is a guess.
*   **Connectors at edges** facing the way the cable actually leaves. The sensor boards moved their mounting holes twice ("relocate pcb holes", "align m3 left side offset").
*   **Mounting holes** ≥ 3 mm from the edge, on a round-number grid, with the same pattern repeated across boards where possible.
*   **Silkscreen:** net names on every connector pin, a board name and revision, a `v0.x` the fab will print.

## 11. Autorouter output is a draft

A router does signals. **Switching-regulator loops, LED current paths and ground returns are drawn by hand first**, then the router gets the rest, then a human looks at every machine-made trace once. Freerouting's own log can list pads as unrouted that KiCad's connectivity shows joined; KiCad is the truth.

## 12. Gates that refuse, not warn

*   **ERC and DRC at zero** before any export. Unrouted nets are an error at export, not at order.
*   The export tool **refuses** a board with unrouted connections, DRC errors, footprint/schematic mismatches, or BOM lines without a part number.
*   DNP parts are marked DNP in the schematic (KiCad's DNP flag), excluded from the BOM export, and their footprints stay on the board.

## 13. At the order

*   Re-check **stock** on every Extended part the day of the order; stock moves.
*   JLCPCB assembles a minimum of 2 boards; **order spares** (5 boards, 2–3 assembled) so a bring-up mistake isn't a re-order.
*   Open the fab's **DFM report** and the assembly preview (§4) before paying.
*   Through-hole connectors can be left for hand soldering if the fab's through-hole assembly is slow or costly; it's ten minutes with a good iron.

---

## Checklists

### Gate A0 → A1: before schematic capture
- [ ] Fab chosen (Q8 answered).
- [ ] Every BOM line has LCSC code, MPN, package, Basic/Extended, stock, datasheet link (§1).
- [ ] Footprint library built from those parts; every footprint's pin map checked against its datasheet drawing (§3).
- [ ] Every MPN opened on the supplier page (§2).
- [ ] Transient questions in §7 answered in the design doc.
- [ ] Connector pin orders chosen for fail-safe adjacent shorts (§7, §8).
- [ ] Enclosure gives board outlines, hole patterns, connector edges and max heights (§10).

### Gate A1 → order
- [ ] Review doc for this revision written, with "checked and left alone" (§6).
- [ ] ERC 0, DRC 0, no unrouted nets; export tool passes (§12).
- [ ] Fiducials, origin, numbered connectors with names on silk, board name + rev on silk (§4, §8, §10).
- [ ] Hand-drawn power paths, human look at every autorouted trace (§11).
- [ ] Fab assembly preview checked for rotation on every polarized part (§4).
- [ ] Stock re-checked; spares in the order (§13).
- [ ] Wiring sheet for every cable; critical leads made by hand (§8).
