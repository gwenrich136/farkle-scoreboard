#ifndef MEMORY_CARD_H
#define MEMORY_CARD_H

#include <SD.h>
#include <vector>
#include <string>

// Maximum number of players stored in memory
#define MAX_POOL_PLAYERS 50

enum class PlayerState {
    AVAILABLE,
    UNUSED,
    SELECTED,
    DELETED
};

struct GameState; // Forward declaration

struct PlayerRecord {
    char name[13];
    uint32_t frequency;
    PlayerState state;
};

struct TurnRecord {
    static uint32_t pack(int score, uint8_t playerIndex, uint8_t farkleCount, bool finalRound, bool penalty) {
        uint32_t record = 0;
        record |= (score & 0xFFFFF); // 20 bits
        record |= ((playerIndex & 0xF) << 20); // 4 bits
        record |= ((farkleCount & 0x3) << 24); // 2 bits
        record |= ((finalRound ? 1 : 0) << 26); // 1 bit
        record |= ((penalty ? 1 : 0) << 27); // 1 bit
        return record;
    }
};

class MemoryCard {
public:
    MemoryCard(int csPin);

    // Core setup
    bool begin();

    // Pre-game selection API
    void beginPlayerSelection();
    const char* getNextPlayer();
    const char* getPreviousPlayer();
    const char* getCurrentPlayer();
    void reservePlayer(char* dest);
    void finalizeSelection();

    // Game Lifecycle Management
    uint32_t getOrGenerateNextGameId();
    void setActiveGameId(uint32_t id);
    void initializeGameDirectory(uint32_t id);
    void writeGameMetadata(uint32_t gameId, const GameState& state);
    void appendTurnRecord(uint32_t record);
    void finalizeGame(const GameState& state);

    // Game Recovery API
    bool hasActiveGame();
    bool loadGameMetadata(GameState& state);
    bool replayGameJournal(GameState& state);
    void clearActiveGame();

    struct UndoResult {
        bool success;        // false if journal is empty (nothing to undo)
        uint8_t playerIndex; // Which player's turn was removed
        int previousScore;   // Their score before that turn (0 if first turn)
        uint8_t previousFarkleCount; // Their farkle count before that turn
    };
    UndoResult undoLastTurn();

private:
    int _csPin;
    uint32_t _activeGameId;
    PlayerRecord _playerPool[MAX_POOL_PLAYERS];
    int _currentIndex;

    void _autopopulateFile();
    void _readPlayersFile();
    void _sortPool();

    // Journal helpers — shared low-level operations on journal.bin
    void _getJournalPath(char* buf, size_t size) const;
    bool _readLastRecord(uint32_t& outRecord) const;
    void _truncateLastJournalRecord();
};

#endif
