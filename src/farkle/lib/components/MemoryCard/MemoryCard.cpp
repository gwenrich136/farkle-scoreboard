#include "MemoryCard.h"
#include <Arduino.h>
#include <string.h>
#include <ArduinoJson.h>
#include "GameState.h"

MemoryCard::MemoryCard(int csPin) : _csPin(csPin), _activeGameId(0), _currentIndex(-1) {
    for (int i = 0; i < MAX_POOL_PLAYERS; i++) {
        _playerPool[i].name[0] = '\0';
        _playerPool[i].frequency = 0;
        _playerPool[i].state = PlayerState::UNUSED;
    }
}

bool MemoryCard::begin() {
    if (!SD.begin(_csPin)) {
        return false;
    }

    if (!SD.exists("players.csv")) {
        _autopopulateFile();
    }

    return true;
}

void MemoryCard::_autopopulateFile() {
    File f = SD.open("players.csv", FILE_WRITE);
    if (f) {
        const char* defaultNames[] = {
            "Geewee", "Sammy", "Coach", "Sheshe",
            "Alex", "Tigre", "Pepa", "Fred", "Andrea"
        };
        for (int i = 0; i < 9; i++) {
            f.print(defaultNames[i]);
            f.println(",0");
        }
        f.close();
    }
}

void MemoryCard::_readPlayersFile() {
    for (int i = 0; i < MAX_POOL_PLAYERS; i++) {
        _playerPool[i].state = PlayerState::UNUSED;
        _playerPool[i].name[0] = '\0';
        _playerPool[i].frequency = 0;
    }

    File f = SD.open("players.csv", FILE_READ);
    if (!f) return;

    int index = 0;
    while (f.available() && index < MAX_POOL_PLAYERS) {
#ifdef UNIT_TEST
        std::string line = f.readStringUntil('\n');
#else
        std::string line = f.readStringUntil('\n').c_str();
#endif

        // Basic trim (Windows CRLF issue handling)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.length() == 0) continue;

        size_t commaIdx = line.find(',');
        if (commaIdx != std::string::npos && commaIdx > 0) {
            std::string name = line.substr(0, commaIdx);
            std::string freqStr = line.substr(commaIdx + 1);

            strncpy(_playerPool[index].name, name.c_str(), 12);
            _playerPool[index].name[12] = '\0';
            _playerPool[index].frequency = std::stoul(freqStr);
            _playerPool[index].state = PlayerState::AVAILABLE;

            index++;
        }
    }
    f.close();
}

void MemoryCard::_sortPool() {
    // Insertion sort descending by frequency
    for (int i = 1; i < MAX_POOL_PLAYERS; i++) {
        PlayerRecord key = _playerPool[i];
        int j = i - 1;

        // Condition to move elements right:
        // 1. If key is AVAILABLE and element j is UNUSED (AVAILABLE goes before UNUSED)
        // 2. If both are AVAILABLE and key frequency is greater than element j frequency
        while (j >= 0 && (
                (_playerPool[j].state == PlayerState::UNUSED && key.state == PlayerState::AVAILABLE) ||
                (_playerPool[j].state == PlayerState::AVAILABLE && key.state == PlayerState::AVAILABLE && _playerPool[j].frequency < key.frequency)
              )) {
            _playerPool[j + 1] = _playerPool[j];
            j = j - 1;
        }
        _playerPool[j + 1] = key;
    }
}

void MemoryCard::beginPlayerSelection() {
    _readPlayersFile();
    _sortPool();

    // Find first available
    _currentIndex = -1;
    // Set to first available so we have a valid selection initially
    for (int i = 0; i < MAX_POOL_PLAYERS; i++) {
        if (_playerPool[i].state == PlayerState::AVAILABLE) {
            _currentIndex = i;
            break;
        }
    }
}

const char* MemoryCard::getCurrentPlayer() {
    if (_currentIndex >= 0 && _currentIndex < MAX_POOL_PLAYERS && _playerPool[_currentIndex].state == PlayerState::AVAILABLE) {
        return _playerPool[_currentIndex].name;
    }
    return "";
}

const char* MemoryCard::getNextPlayer() {
    int nextIdx = _currentIndex + 1;
    while (nextIdx < MAX_POOL_PLAYERS) {
        if (_playerPool[nextIdx].state == PlayerState::AVAILABLE) {
            _currentIndex = nextIdx;
            return _playerPool[_currentIndex].name;
        }
        nextIdx++;
    }

    // According to design doc: "scrolling out of bounds updates the cursor to special indices
    // (START_OF_LIST -1, or END_OF_LIST 50) and returns an empty string"
    _currentIndex = MAX_POOL_PLAYERS;
    return ""; // Hit end of list
}

const char* MemoryCard::getPreviousPlayer() {
    int prevIdx = _currentIndex - 1;
    if (_currentIndex == MAX_POOL_PLAYERS) prevIdx = MAX_POOL_PLAYERS - 1;

    while (prevIdx >= 0) {
        if (_playerPool[prevIdx].state == PlayerState::AVAILABLE) {
            _currentIndex = prevIdx;
            return _playerPool[_currentIndex].name;
        }
        prevIdx--;
    }

    _currentIndex = -1;
    return ""; // Hit start of list
}

void MemoryCard::reservePlayer(char* dest) {
    if (_currentIndex >= 0 && _currentIndex < MAX_POOL_PLAYERS &&
        _playerPool[_currentIndex].state == PlayerState::AVAILABLE) {

        _playerPool[_currentIndex].state = PlayerState::SELECTED;
        strncpy(dest, _playerPool[_currentIndex].name, 12);
        dest[12] = '\0';

        // Auto-advance cursor to next available, or previous if none
        const char* next = getNextPlayer();
        if (next[0] == '\0') {
             getPreviousPlayer();
        }
    }
}

void MemoryCard::finalizeSelection() {
    SD.remove("players.csv");

    File f = SD.open("players.csv", FILE_WRITE);
    if (!f) return;

    // Re-collect active players
    for (int i = 0; i < MAX_POOL_PLAYERS; i++) {
        if (_playerPool[i].state == PlayerState::AVAILABLE || _playerPool[i].state == PlayerState::SELECTED) {
            if (_playerPool[i].state == PlayerState::SELECTED) {
                _playerPool[i].frequency++;
                _playerPool[i].state = PlayerState::AVAILABLE; // Reset for next game
            }
            f.print(_playerPool[i].name);
            f.print(",");

            // Standard formatting for embedded environments avoiding std::to_string issues
            char freqStr[16];
            snprintf(freqStr, sizeof(freqStr), "%lu", (unsigned long)_playerPool[i].frequency);
            f.println(freqStr);
        }
    }
    f.close();
}

uint32_t MemoryCard::getOrGenerateNextGameId() {
    uint32_t nextId = 1;
    if (SD.exists("/sys/next_id.txt")) {
        File f = SD.open("/sys/next_id.txt", FILE_READ);
        if (f) {
            String idStr = f.readStringUntil('\n');
            nextId = idStr.toInt();
            f.close();
        }
    } else if (!SD.exists("/sys")) {
        SD.mkdir("/sys");
    }

    // Increment and save
    SD.remove("/sys/next_id.txt");
    File fOut = SD.open("/sys/next_id.txt", FILE_WRITE);
    if (fOut) {
        char idBuf[16];
        snprintf(idBuf, sizeof(idBuf), "%08lu", (unsigned long)(nextId + 1));
        fOut.println(idBuf);
        fOut.close();
    }
    return nextId;
}

bool MemoryCard::hasActiveGame() {
    if (SD.exists("/sys/curr_id.txt")) {
        File f = SD.open("/sys/curr_id.txt", FILE_READ);
        if (f) {
            String idStr = f.readStringUntil('\n');
            _activeGameId = idStr.toInt();
            f.close();
            return _activeGameId > 0;
        }
    }
    return false;
}

void MemoryCard::clearActiveGame() {
    if (_activeGameId > 0) {
        char metaPath[64];
        snprintf(metaPath, sizeof(metaPath), "/partial/%08lu/meta.jsn", (unsigned long)_activeGameId);
        char dirPath[64];
        snprintf(dirPath, sizeof(dirPath), "/partial/%08lu", (unsigned long)_activeGameId);
        char partialJournalPath[64];
        _getJournalPath(partialJournalPath, sizeof(partialJournalPath));

        SD.remove(metaPath);
        SD.remove(partialJournalPath);
        SD.rmdir(dirPath);
        SD.remove("/sys/curr_id.txt");
        _activeGameId = 0;
    }
}

bool MemoryCard::loadGameMetadata(GameState& state) {
    if (_activeGameId == 0) return false;

    char filePath[64];
    snprintf(filePath, sizeof(filePath), "/partial/%08lu/meta.jsn", (unsigned long)_activeGameId);

    File f = SD.open(filePath, FILE_READ);
    if (!f) return false;

    // Use a reasonable buffer size for 16 players
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, f);
    f.close();

    if (error) {
        return false;
    }

    state.reset();

    if (doc.containsKey("targetScore")) {
        state.targetScore = doc["targetScore"];
    }

    JsonArray players = doc["players"].as<JsonArray>();
    for (JsonObject pObj : players) {
        std::string name = pObj["name"].as<std::string>();
        uint16_t hue = pObj["hue"].as<uint16_t>();
        state.players.push_back(Player(name, hue));
    }

    return true;
}

bool MemoryCard::replayGameJournal(GameState& state) {
    if (_activeGameId == 0 || state.players.empty()) return false;

    char filePath[64];
    _getJournalPath(filePath, sizeof(filePath));

    File f = SD.open(filePath, FILE_READ);
    if (!f) {
        // It's possible the journal doesn't exist yet if we crashed immediately after player selection.
        // That's fine, we just start at player 0 with 0 scores.
        state.currentPlayerIndex = 0;
        return true;
    }

    uint32_t record;
    int lastPlayerIndex = -1;

    // We update scores directly on the state. Wait, the state structure says:
    // "Resume: Read the file from start to finish. For each record, update the corresponding player's score and farkle count in GameState. The last record in the file determines whose turn was just completed; the next player in the meta.jsn sequence is the current active player."

    while (f.read((uint8_t*)&record, sizeof(record)) == sizeof(record)) {
        int      score       = record & 0xFFFFF;
        uint8_t  playerIdx   = (record >> 20) & 0xF;
        uint8_t  farkleCount = (record >> 24) & 0x3;
        // bool     finalRound  = (record >> 26) & 0x1;
        // bool     penalty     = (record >> 27) & 0x1;

        if (playerIdx < state.players.size()) {
            state.updatePlayerScore(playerIdx, score);
            state.players[playerIdx].farkle_count = farkleCount;
            lastPlayerIndex = playerIdx;
        }
    }
    f.close();

    if (lastPlayerIndex >= 0) {
        state.currentPlayerIndex = (lastPlayerIndex + 1) % state.players.size();
    } else {
        state.currentPlayerIndex = 0;
    }

    return true;
}

void MemoryCard::setActiveGameId(uint32_t id) {
    _activeGameId = id;
    if (!SD.exists("/sys")) {
        SD.mkdir("/sys");
    }
    SD.remove("/sys/curr_id.txt");
    File f = SD.open("/sys/curr_id.txt", FILE_WRITE);
    if (f) {
        char idBuf[16];
        snprintf(idBuf, sizeof(idBuf), "%08lu", (unsigned long)id);
        f.println(idBuf);
        f.flush();
        f.close();
    }
}

void MemoryCard::initializeGameDirectory(uint32_t id) {
    char dirPath[32];
    snprintf(dirPath, sizeof(dirPath), "/partial/%08lu", (unsigned long)id);
    if (!SD.exists("/partial")) {
        SD.mkdir("/partial");
    }
    if (!SD.exists(dirPath)) {
        SD.mkdir(dirPath);
    }
}

void MemoryCard::writeGameMetadata(uint32_t gameId, const GameState& state) {
    char filePath[64];
    snprintf(filePath, sizeof(filePath), "/partial/%08lu/meta.jsn", (unsigned long)gameId);

    SD.remove(filePath);
    File f = SD.open(filePath, FILE_WRITE);
    if (!f) return;

    JsonDocument doc;
    doc["targetScore"] = state.targetScore;
    JsonArray players = doc["players"].to<JsonArray>();
    
    for (const auto& player : state.players) {
        JsonObject pObj = players.add<JsonObject>();
        pObj["name"] = player.name;
        pObj["hue"] = player.hue;
    }

    serializeJson(doc, f);
    f.flush();
    f.close();
}

// --- Journal Helpers ---

void MemoryCard::_getJournalPath(char* buf, size_t size) const {
    snprintf(buf, size, "/partial/%08lu/journal.bin", (unsigned long)_activeGameId);
}

// Reads the last 4-byte record from journal.bin into outRecord.
// Returns false if the file doesn't exist, is empty, or a read error occurs.
bool MemoryCard::_readLastRecord(uint32_t& outRecord) const {
    char filePath[64];
    _getJournalPath(filePath, sizeof(filePath));

    File f = SD.open(filePath, FILE_READ);
    if (!f) return false;

    uint32_t fileSize = f.size();
    if (fileSize < sizeof(uint32_t)) { f.close(); return false; }

    f.seek(fileSize - sizeof(uint32_t));
    bool ok = (f.read((uint8_t*)&outRecord, sizeof(outRecord)) == sizeof(outRecord));
    f.close();
    return ok;
}

// Removes the last 4 bytes from journal.bin by rewriting via a temp file.
// SD libraries don't support in-place truncation, so we use a copy-rename idiom.
void MemoryCard::_truncateLastJournalRecord() {
    char filePath[64];
    _getJournalPath(filePath, sizeof(filePath));

    char tempPath[64];
    snprintf(tempPath, sizeof(tempPath), "/partial/%08lu/journal.tmp", (unsigned long)_activeGameId);

    // Pass 1: copy all records except the last into a temp file
    {
        File src = SD.open(filePath, FILE_READ);
        File dst = SD.open(tempPath, FILE_WRITE);
        if (src && dst) {
            uint32_t bytesToCopy = src.size();
            if (bytesToCopy >= sizeof(uint32_t)) {
                bytesToCopy -= sizeof(uint32_t);
            }
            uint32_t bytesWritten = 0;
            uint32_t rec;
            while (bytesWritten + sizeof(uint32_t) <= bytesToCopy &&
                   src.read((uint8_t*)&rec, sizeof(rec)) == sizeof(rec)) {
                dst.write((const uint8_t*)&rec, sizeof(rec));
                bytesWritten += sizeof(uint32_t);
            }
        }
        if (src) src.close();
        if (dst) dst.close();
    }

    // Pass 2: replace original with temp
    SD.remove(filePath);
    {
        File tmpRead  = SD.open(tempPath, FILE_READ);
        File jnlWrite = SD.open(filePath, FILE_WRITE);
        if (tmpRead && jnlWrite) {
            uint32_t rec;
            while (tmpRead.read((uint8_t*)&rec, sizeof(rec)) == sizeof(rec)) {
                jnlWrite.write((const uint8_t*)&rec, sizeof(rec));
            }
        }
        if (tmpRead)  tmpRead.close();
        if (jnlWrite) jnlWrite.close();
    }
    SD.remove(tempPath);
}

// --- Public Lifecycle API ---

void MemoryCard::appendTurnRecord(uint32_t record) {
    if (_activeGameId == 0) return;

    char filePath[64];
    _getJournalPath(filePath, sizeof(filePath));

    File f = SD.open(filePath, FILE_WRITE);
    if (!f) return;

    // Write the 32-bit integer in little-endian format
    f.write((const uint8_t*)&record, sizeof(record));
    f.flush();
    f.close();
}

void MemoryCard::finalizeGame(const GameState& state) {
    if (_activeGameId == 0) return;

    if (!SD.exists("/archive")) {
        SD.mkdir("/archive");
    }

    char partialJournalPath[64];
    _getJournalPath(partialJournalPath, sizeof(partialJournalPath));

    char archiveCsvPath[64];
    snprintf(archiveCsvPath, sizeof(archiveCsvPath), "/archive/%08lu.csv", (unsigned long)_activeGameId);

    // Decode journal.bin into a human-readable CSV
    File journal = SD.open(partialJournalPath, FILE_READ);
    File csv     = SD.open(archiveCsvPath, FILE_WRITE);

    if (journal && csv) {
        csv.println("Turn,Player,Score,Farkles,FinalRound,Penalty");
        int turnNum = 1;
        uint32_t record;
        while (journal.read((uint8_t*)&record, sizeof(record)) == sizeof(record)) {
            int      score      = record & 0xFFFFF;
            uint8_t  playerIdx  = (record >> 20) & 0xF;
            uint8_t  farkleCount = (record >> 24) & 0x3;
            bool     finalRound = (record >> 26) & 0x1;
            bool     penalty    = (record >> 27) & 0x1;

            const char* name = (playerIdx < state.players.size())
                ? state.players[playerIdx].name.c_str() : "Unknown";

            csv.print(turnNum++); csv.print(",");
            csv.print(name);      csv.print(",");
            csv.print(score);     csv.print(",");
            csv.print(farkleCount); csv.print(",");
            csv.print(finalRound ? 1 : 0); csv.print(",");
            csv.println(penalty ? 1 : 0);
        }
    }
    if (journal) journal.close();
    if (csv) {
        csv.flush();
        csv.close();
    }

    // Clean up partial directory
    char metaPath[64];
    snprintf(metaPath, sizeof(metaPath), "/partial/%08lu/meta.jsn", (unsigned long)_activeGameId);
    char dirPath[64];
    snprintf(dirPath, sizeof(dirPath), "/partial/%08lu", (unsigned long)_activeGameId);

    SD.remove(metaPath);
    SD.remove(partialJournalPath);
    SD.rmdir(dirPath);
    SD.remove("/sys/curr_id.txt");

    _activeGameId = 0; // Game is finalized
}

MemoryCard::UndoResult MemoryCard::undoLastTurn() {
    UndoResult result = {false, 0, 0, 0};
    if (_activeGameId == 0) return result;

    // Step 1: Read the last record to identify which player's turn to remove
    uint32_t lastRecord;
    if (!_readLastRecord(lastRecord)) return result; // Empty or missing journal

    uint8_t targetPlayer = (lastRecord >> 20) & 0xF;

    // Step 2: Remove the last record from the journal
    _truncateLastJournalRecord();

    // Step 3: Scan backwards for the most recent prior record for targetPlayer
    char filePath[64];
    _getJournalPath(filePath, sizeof(filePath));

    File scan = SD.open(filePath, FILE_READ);
    if (scan) {
        int32_t pos = (int32_t)scan.size() - sizeof(uint32_t);
        while (pos >= 0) {
            scan.seek((uint32_t)pos);
            uint32_t rec;
            scan.read((uint8_t*)&rec, sizeof(rec));
            if (((rec >> 20) & 0xF) == targetPlayer) {
                result.previousScore      = rec & 0xFFFFF;
                result.previousFarkleCount = (rec >> 24) & 0x3;
                break;
            }
            pos -= sizeof(uint32_t);
        }
        scan.close();
    }
    // If no prior record found, previousScore/Farkles stay at 0 (first-ever turn)

    result.success     = true;
    result.playerIndex = targetPlayer;
    return result;
}
