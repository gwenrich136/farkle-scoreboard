#include "MemoryCard.h"
#include <Arduino.h>
#include <string.h>

MemoryCard::MemoryCard(int csPin) : _csPin(csPin), _currentIndex(-1) {
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
