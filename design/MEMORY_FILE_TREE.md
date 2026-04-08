# Memory File Tree & Data Specification

This document defines the file structure and binary data formats used by the `MemoryCard` component for persistent storage on the Farkle Scoreboard.

---

## 1. Directory Structure

The SD card is organized into three main areas: system configuration, the global player pool, and game-specific data folders.

```text
/players.txt               # Global pool of unique player names (one per line)
/sys/
    ├── next_id.txt        # 8-digit decimal ID for the next game (e.g., 00000042)
    └── last_active.txt    # 8-digit decimal ID of the most recent un-finalized game
/partial/                  # Folders for games currently in progress
    └── [ID]/              # e.g., /partial/00000042/
        ├── meta.csv       # Game configuration (Target, Players, Hues)
        └── journal.bin    # 32-bit raw binary turn records
/completed/                # Folders for finished games (moved from /partial)
    └── [ID]/
        ├── meta.csv
        ├── journal.bin
        └── summary.csv    # (Optional) Human-readable turn-by-turn breakdown
```

---

## 2. Global Player Pool (`players.txt`)
- **Format:** Plain text, UTF-8.
- **Content:** One name per line.
- **Constraints:** Max 50 names, max 12 characters per name.
- **Indexing:** The `MemoryCard` loads these into RAM and refers to them by their 0-based line index.

---

## 3. Game Metadata (`meta.csv`)
Each game folder contains a `meta.csv` that defines the "rules" and "players" for that specific session. This ensures the game remains resume-able even if the global `players.txt` is modified.

**Schema:**
`TargetScore, PlayerCount, [Name1, Hue1], [Name2, Hue2] ...`

**Example:**
`10000, 3, "Sammy", 42000, "Coach", 12000, "Alex", 55000`

---

## 4. The Journal Record (`journal.bin`)
The `journal.bin` is an append-only file where every turn is recorded as a single **32-bit unsigned integer**. This format is optimized for the Uno R4's 32-bit architecture.

### Bit-Packing Specification (32-bit)

| Bits | Width | Field | Description |
| :--- | :--- | :--- | :--- |
| **0-19** | 20 | **Score** | Literal banked score (Max: 1,048,575). |
| **20-23** | 4 | **Player Index** | The 0-based index of the player (0-15). |
| **24-25** | 2 | **Farkle Count** | Consecutive farkles at end of turn (0, 1, or 2). |
| **26** | 1 | **Final Round** | Set to 1 if this turn triggered the Final Round. |
| **27** | 1 | **Penalty** | Set to 1 if this turn resulted in a -1000 penalty. |
| **28-31** | 4 | **Reserved** | Future flags (e.g., "Undo Triggered", "Bonus Roll"). |

### Operations:
- **Append:** Write 4 bytes to the end of the file.
- **Undo:** Truncate the file by exactly 4 bytes.
- **Resume:** Read the file from start to finish. For each record, update the corresponding player's score and farkle count in `GameState`. The last record in the file determines whose turn was just completed.
- **Snapshot Preview:** Jump to the end of the file and read back `N` records (where `N` = PlayerCount) to determine the current score spread for the "Live Preview" UI.

---

## 5. Data Integrity & Recovery
- **Torn Write Protection:** On boot, the `MemoryCard` component checks if `journal.bin` size is a multiple of 4. If not, it truncates the file to the last 4-byte boundary.
- **Atomic Finalization:** When a game is won, the `/partial/[ID]` folder is moved to `/completed/[ID]` and `sys/last_active.txt` is updated or cleared.
- **Self-Healing:** If `/sys/next_id.txt` is missing, the component scans both `/partial` and `/completed` for the highest existing ID and increments from there.
