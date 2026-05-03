#ifndef FAKE_MEMORY_CARD_H
#define FAKE_MEMORY_CARD_H

#include <vector>
#include <string>

// Maximum number of players stored in memory
#define MAX_POOL_PLAYERS 50

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

    // Mock specific
    void _setMockPlayers(const std::vector<std::string>& players);

private:
    int _csPin;
    std::vector<std::string> _mockPlayers;
    int _currentIndex;
};

#endif
