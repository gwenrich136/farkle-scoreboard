#include "MemoryCard.h"
#include <string.h>
#include <stdlib.h>

#define START_OF_LIST -1
#define END_OF_LIST MAX_POOL_SIZE

const char* const DEFAULT_NAMES[] = {
    "Geewee", "Sammy", "Coach", "Sheshe", "Alex", "Tigre", "Pepa", "Fred", "Andrea"
};

MemoryCard::MemoryCard(int csPin) : _csPin(csPin), _currentIndex(START_OF_LIST) {
    clearPool();
}

bool MemoryCard::begin() {
    if (!SD.begin(_csPin)) {
        return false;
    }

    if (!SD.exists("players.csv")) {
        createDefaultPlayersFile();
    }
    return true;
}

void MemoryCard::createDefaultPlayersFile() {
    File file = SD.open("players.csv", FILE_WRITE);
    if (file) {
        for (size_t i = 0; i < sizeof(DEFAULT_NAMES) / sizeof(DEFAULT_NAMES[0]); ++i) {
            file.print(DEFAULT_NAMES[i]);
            file.println(",0");
        }
        file.close();
    }
}

void MemoryCard::beginPlayerSelection() {
    loadPlayersFromCSV();
    sortPool();
    _currentIndex = START_OF_LIST;
}

void MemoryCard::resetCursor() {
    _currentIndex = START_OF_LIST;
}

void MemoryCard::clearPool() {
    for (int i = 0; i < MAX_POOL_SIZE; ++i) {
        _pool[i].name[0] = '\0';
        _pool[i].state = PlayerState::UNUSED;
        _pool[i].frequency = 0;
    }
}

void MemoryCard::loadPlayersFromCSV() {
    clearPool();

    File file = SD.open("players.csv", FILE_READ);
    if (!file) {
        return;
    }

    int idx = 0;
    while (file.available() && idx < MAX_POOL_SIZE) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        int commaPos = line.indexOf(',');
        if (commaPos != -1) {
            String namePart = line.substring(0, commaPos);
            String freqPart = line.substring(commaPos + 1);

            strncpy(_pool[idx].name, namePart.c_str(), MAX_NAME_LEN);
            _pool[idx].name[MAX_NAME_LEN] = '\0';
            _pool[idx].frequency = freqPart.toInt();
            _pool[idx].state = PlayerState::AVAILABLE;
            idx++;
        }
    }
    file.close();
}

void MemoryCard::sortPool() {
    // Simple bubble sort since MAX_POOL_SIZE is small (50)
    for (int i = 0; i < MAX_POOL_SIZE - 1; ++i) {
        for (int j = 0; j < MAX_POOL_SIZE - i - 1; ++j) {
            // Keep unused at the end
            if (_pool[j].state == PlayerState::UNUSED && _pool[j+1].state != PlayerState::UNUSED) {
                PlayerRecord temp = _pool[j];
                _pool[j] = _pool[j+1];
                _pool[j+1] = temp;
            } else if (_pool[j].state != PlayerState::UNUSED && _pool[j+1].state != PlayerState::UNUSED) {
                if (_pool[j].frequency < _pool[j+1].frequency) {
                    PlayerRecord temp = _pool[j];
                    _pool[j] = _pool[j+1];
                    _pool[j+1] = temp;
                }
            }
        }
    }
}

const char* MemoryCard::getNextPlayer() {
    int nextIndex = _currentIndex + 1;
    while (nextIndex < MAX_POOL_SIZE) {
        if (_pool[nextIndex].state == PlayerState::AVAILABLE) {
            _currentIndex = nextIndex;
            return _pool[_currentIndex].name;
        }
        nextIndex++;
    }
    _currentIndex = END_OF_LIST;
    return "";
}

const char* MemoryCard::getPreviousPlayer() {
    int prevIndex = _currentIndex - 1;
    while (prevIndex >= 0) {
        if (_pool[prevIndex].state == PlayerState::AVAILABLE) {
            _currentIndex = prevIndex;
            return _pool[_currentIndex].name;
        }
        prevIndex--;
    }
    _currentIndex = START_OF_LIST;
    return "";
}

const char* MemoryCard::getCurrentPlayer() {
    if (_currentIndex >= 0 && _currentIndex < MAX_POOL_SIZE) {
        if (_pool[_currentIndex].state == PlayerState::AVAILABLE) {
            return _pool[_currentIndex].name;
        }
    }
    return "";
}

void MemoryCard::reservePlayer(char* locationToCopy) {
    if (_currentIndex >= 0 && _currentIndex < MAX_POOL_SIZE) {
        if (_pool[_currentIndex].state == PlayerState::AVAILABLE) {
            _pool[_currentIndex].state = PlayerState::SELECTED;
            strncpy(locationToCopy, _pool[_currentIndex].name, MAX_NAME_LEN);
            locationToCopy[MAX_NAME_LEN] = '\0';
            return;
        }
    }

    locationToCopy[0] = '\0';
}

void MemoryCard::savePlayersToCSV() {
    // Delete the file to recreate it
    if (SD.exists("players.csv")) {
        SD.remove("players.csv");
    }

    File file = SD.open("players.csv", FILE_WRITE);
    if (!file) return;

    for (int i = 0; i < MAX_POOL_SIZE; ++i) {
        if (_pool[i].state == PlayerState::AVAILABLE || _pool[i].state == PlayerState::SELECTED) {
            file.print(_pool[i].name);
            file.print(",");
            file.println(_pool[i].frequency);
        }
    }
    file.close();
}

void MemoryCard::finalizeSelection() {
    for (int i = 0; i < MAX_POOL_SIZE; ++i) {
        if (_pool[i].state == PlayerState::SELECTED) {
            _pool[i].frequency++;
        }
    }
    sortPool();
    savePlayersToCSV();
}
