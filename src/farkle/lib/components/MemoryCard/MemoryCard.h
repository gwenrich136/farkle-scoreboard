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

struct PlayerRecord {
    char name[13];
    uint32_t frequency;
    PlayerState state;
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

private:
    int _csPin;
    PlayerRecord _playerPool[MAX_POOL_PLAYERS];
    int _currentIndex;

    void _autopopulateFile();
    void _readPlayersFile();
    void _sortPool();
};

#endif
