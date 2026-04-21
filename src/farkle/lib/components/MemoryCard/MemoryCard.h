#ifndef MemoryCard_h
#define MemoryCard_h

#include <Arduino.h>

#ifdef UNIT_TEST
#include "SD.h"
#else
#include <SD.h>
#endif

#define MAX_POOL_SIZE 50
#define MAX_NAME_LEN 12

enum class PlayerState {
    AVAILABLE,
    UNUSED,
    SELECTED,
    DELETED
};

struct PlayerRecord {
    char name[MAX_NAME_LEN + 1];
    PlayerState state;
    uint32_t frequency;
};

class MemoryCard {
public:
    MemoryCard(int csPin);

    bool begin();

    // Pre-Game Player Management API
    void beginPlayerSelection();
    void resetCursor();
    const char* getNextPlayer();
    const char* getPreviousPlayer();
    const char* getCurrentPlayer();
    void reservePlayer(char* locationToCopy);
    void finalizeSelection();

private:
    int _csPin;
    PlayerRecord _pool[MAX_POOL_SIZE];
    int _currentIndex;

    void createDefaultPlayersFile();
    void loadPlayersFromCSV();
    void savePlayersToCSV();
    void sortPool();
};

#endif
