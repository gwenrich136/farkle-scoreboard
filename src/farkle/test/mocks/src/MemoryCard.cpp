#include "MemoryCard.h"
#include <string.h>

MemoryCard::MemoryCard(int csPin) : _csPin(csPin), _currentIndex(-1) {
    const char* defaultNames[] = {
        "Geewee", "Sammy", "Coach", "Sheshe",
        "Alex", "Tigre", "Pepa", "Fred", "Andrea"
    };
    for (int i = 0; i < 9; i++) {
        _mockPlayers.push_back(defaultNames[i]);
    }
}

bool MemoryCard::begin() {
    return true;
}

void MemoryCard::_setMockPlayers(const std::vector<std::string>& players) {
    _mockPlayers = players;
}

void MemoryCard::beginPlayerSelection() {
    _currentIndex = _mockPlayers.empty() ? -1 : 0;
}

const char* MemoryCard::getCurrentPlayer() {
    if (_currentIndex >= 0 && _currentIndex < (int)_mockPlayers.size()) {
        return _mockPlayers[_currentIndex].c_str();
    }
    return "";
}

const char* MemoryCard::getNextPlayer() {
    if (_currentIndex < (int)_mockPlayers.size()) {
        _currentIndex++;
    }
    if (_currentIndex >= (int)_mockPlayers.size()) {
        _currentIndex = (int)_mockPlayers.size(); // Set to out of bounds
        return "";
    }
    return _mockPlayers[_currentIndex].c_str();
}

const char* MemoryCard::getPreviousPlayer() {
    if (_currentIndex >= 0) {
        _currentIndex--;
    }
    if (_currentIndex < 0) {
        _currentIndex = -1; // Set to out of bounds
        return "";
    }
    return _mockPlayers[_currentIndex].c_str();
}

void MemoryCard::reservePlayer(char* dest) {
    if (_currentIndex >= 0 && _currentIndex < (int)_mockPlayers.size()) {
        strncpy(dest, _mockPlayers[_currentIndex].c_str(), 12);
        dest[12] = '\0';
        
        // Remove from available players (mock reservation behavior)
        _mockPlayers.erase(_mockPlayers.begin() + _currentIndex);

        // Auto-advance
        if (_mockPlayers.empty()) {
            _currentIndex = -1;
        } else if (_currentIndex >= (int)_mockPlayers.size()) {
            _currentIndex = (int)_mockPlayers.size() - 1; // Fall back to previous (which is now the last item)
        }
    }
}

void MemoryCard::finalizeSelection() {
    // No-op for mock
}
