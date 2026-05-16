# Memory File Tree & Data Specification

This document defines the file structure and binary data formats used by the `MemoryCard` component for persistent storage on the Farkle Scoreboard.

---

## 1. Directory Structure

The SD card is organized into three main areas: system configuration, the global player pool, and game-specific data folders.

```text
/players.csv               # Global pool of player names and their play frequencies
/sys/
    ├── next_id.txt        # 8-digit decimal ID for the next game (e.g., 00000042)
    └── curr_id.txt        # 8-digit decimal ID of the current un-finalized game
/partial/                  # Folders for games currently in progress
    └── [ID]/              # e.g., /partial/00000042/
        ├── meta.jsn       # Game configuration (Target, Players, Hues, completed flag)
        └── journal.bin    # 32-bit raw binary turn records
/archive/                  # Folders for completed games (same structure as /partial/)
    └── [ID]/              # e.g., /archive/00000042/
        ├── meta.jsn       # Same format as partial, with "completed": true
        └── journal.bin    # Complete turn history (identical binary format)
```

---

## 2. Global Player Pool (`players.csv`)
- **Format:** Plain text, comma-separated values (CSV), UTF-8.
- **Content:** `<Name>,<Frequency>` (e.g., `Sammy,14`).
- **Constraints:** Max 50 records. Name must be max 12 characters. Frequency is an unsigned integer.
- **Sorting:** The `MemoryCard` loads these into a fixed-size RAM buffer and sorts them in descending order based on `Frequency` so that the most frequently played names appear first.
- **Indexing:** While in the selection phase, the system uses a `PlayerState` enum (`AVAILABLE`, `SELECTED`, `UNUSED`, `DELETED`) rather than manipulating the pool index to support reserving and skipping names easily.

---

## 3. Game Metadata (`meta.jsn`)
Each game folder contains a `meta.jsn` that defines the "rules" and "players" for that specific session. This ensures the game remains resume-able even if the global `players.csv` is modified. It uses JSON for extensibility, but uses the `.jsn` extension to comply with the strict FAT32 8.3 file naming constraints (max 3 characters for the extension).

The `completed` field is a boolean that distinguishes partial (in-progress) games from finished ones. It is set to `false` when the game is created and updated to `true` during finalization. This provides an embedded marker of game state that survives directory corruption or accidental file moves.

**Example (in-progress game):**
```json
{
  "targetScore": 10000,
  "completed": false,
  "players": [
    { "name": "Sammy", "hue": 42000 },
    { "name": "Coach", "hue": 12000 },
    { "name": "Alex", "hue": 55000 }
  ]
}
```

**Example (completed game):**
```json
{
  "targetScore": 10000,
  "completed": true,
  "players": [
    { "name": "Sammy", "hue": 42000 },
    { "name": "Coach", "hue": 12000 },
    { "name": "Alex", "hue": 55000 }
  ]
}
```

---

## 4. The Journal Record (`journal.bin`)
The `journal.bin` is an append-only file where every turn is recorded as a single **32-bit unsigned integer**. This format is optimized for the Uno R4's 32-bit architecture.

### Bit-Packing Specification (32-bit)

| Bits | Width | Field | Description |
| :--- | :--- | :--- | :--- |
| **0-19** | 20 | **Score** | Literal banked score (Max: 1,048,575). *Note: Farkle rules ensure total score cannot drop below zero.* |
| **20-23** | 4 | **Player Index** | The 0-based index of the player (0-15). |
| **24-25** | 2 | **Farkle Count** | Consecutive farkles at end of turn (0, 1, or 2). |
| **26** | 1 | **Final Round** | Set to 1 if this turn triggered the Final Round. |
| **27** | 1 | **Penalty** | Set to 1 if this turn resulted in a -1000 penalty. |
| **28-31** | 4 | **Reserved** | Future flags (e.g., "Undo Triggered", "Bonus Roll"). |

### Operations:
- **Append:** Write 4 bytes to the end of the file.
- **Undo:** 
    1. Read the last 4-byte record to identify the player.
    2. Truncate the file by exactly 4 bytes.
    3. Scan the `journal.bin` backwards to find the *most recent* previous record for that same player.
    4. Return that player's previous score and farkle count (or 0 if no prior records exist).
- **Resume:** Read the file from start to finish. For each record, update the corresponding player's score and farkle count in `GameState`. The last record in the file determines whose turn was just completed; the *next* player in the `meta.jsn` sequence is the current active player.
- **Snapshot Preview:** Jump to the end of the file and read backwards 4 bytes at a time. Update the preview score for a player if they haven't been seen yet. Stop reading once a record for all `N` players has been found, or the beginning of the file is reached (handling early-game states where not everyone has taken a turn).

---

## 5. Data Integrity & Recovery
- **Torn Write Protection:** On boot, the `MemoryCard` component checks if `journal.bin` size is a multiple of 4. If not, it truncates the file to the last 4-byte boundary.
- **Finalization (Copy-Mark-Delete):** When a game is won, the finalization process:
    1. Creates the `/archive/[ID]/` directory.
    2. Copies `journal.bin` from `/partial/[ID]/` to `/archive/[ID]/`.
    3. Rewrites `meta.jsn` into `/archive/[ID]/` with `"completed": true`.
    4. Deletes `/partial/[ID]/journal.bin`, `/partial/[ID]/meta.jsn`, and the `/partial/[ID]/` directory.
    5. Deletes `sys/curr_id.txt`.
    6. Resets the internal `_activeGameId` to 0.
    This avoids the lack of atomic directory moves on SD libraries. The `completed` flag in `meta.jsn` provides resilience: if the process is interrupted between steps 3 and 4, the game can be identified as completed even if files exist in both directories.
- **Recovery Flow:** If `sys/curr_id.txt` exists on boot, the `Game` should prompt to "Resume Game" via `StartupPhase`. Upon confirmation, it loads metadata from `meta.jsn` to restore player state and parses `journal.bin` to reconstruct scores and identify the next active player.
- **Self-Healing:** If `/sys/next_id.txt` is missing, the component scans both `/partial` and `/archive` for the highest existing ID and increments from there.
- **Unified Read Path:** Both partial and archived games use the identical `meta.jsn` + `journal.bin` format. The same `loadGameMetadata()` and `replayGameJournal()` methods work for both, with only the base path (`/partial/` vs `/archive/`) differing.
