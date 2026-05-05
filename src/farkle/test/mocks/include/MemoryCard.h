#ifndef FAKE_MEMORY_CARD_H
#define FAKE_MEMORY_CARD_H

#include <vector>
#include <string>
#include <cstdint>

// Maximum number of players stored in memory
#define MAX_POOL_PLAYERS 50

struct GameState;

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

    struct UndoResult {
        bool success;
        uint8_t playerIndex;
        int previousScore;
        uint8_t previousFarkleCount;
    };
    UndoResult undoLastTurn();

    // Mock specific
    void _setMockPlayers(const std::vector<std::string>& players);

    // Spy variables
    std::vector<std::string> mock_call_order;
    bool mock_getOrGenerateNextGameId_called = false;
    bool mock_setActiveGameId_called = false;
    uint32_t mock_setActiveGameId_arg = 0;
    bool mock_initializeGameDirectory_called = false;
    uint32_t mock_initializeGameDirectory_arg = 0;
    bool mock_writeGameMetadata_called = false;
    uint32_t mock_writeGameMetadata_arg = 0;
    bool mock_appendTurnRecord_called = false;
    std::vector<uint32_t> mock_appendTurnRecord_args;
    bool mock_finalizeGame_called = false;
    bool mock_undoLastTurn_called = false;
    int mock_undoLastTurn_call_count = 0;
    UndoResult mock_undoLastTurn_result = {false, 0, 0, 0};

private:
    int _csPin;
    std::vector<std::string> _mockPlayers;
    int _currentIndex;
};

#endif
