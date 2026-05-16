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

uint32_t MemoryCard::getOrGenerateNextGameId() {
    mock_call_order.push_back("getOrGenerateNextGameId");
    mock_getOrGenerateNextGameId_called = true;
    return 42; // mock implementation
}

void MemoryCard::setActiveGameId(uint32_t id) {
    mock_call_order.push_back("setActiveGameId");
    mock_setActiveGameId_called = true;
    mock_setActiveGameId_arg = id;
}

void MemoryCard::initializeGameDirectory(uint32_t id) {
    mock_call_order.push_back("initializeGameDirectory");
    mock_initializeGameDirectory_called = true;
    mock_initializeGameDirectory_arg = id;
}

void MemoryCard::writeGameMetadata(uint32_t gameId, const GameState& state) {
    mock_call_order.push_back("writeGameMetadata");
    mock_writeGameMetadata_called = true;
    mock_writeGameMetadata_arg = gameId;
}

void MemoryCard::appendTurnRecord(uint32_t record) {
    mock_appendTurnRecord_called = true;
    mock_appendTurnRecord_args.push_back(record);
}

void MemoryCard::finalizeGame(const GameState& state) {
    mock_finalizeGame_called = true;
}

MemoryCard::UndoResult MemoryCard::undoLastTurn() {
    mock_undoLastTurn_called = true;
    mock_undoLastTurn_call_count++;
    return mock_undoLastTurn_result;
}

bool MemoryCard::hasActiveGame() {
    mock_hasActiveGame_called = true;
    return mock_hasActiveGame_result;
}

bool MemoryCard::loadGameMetadata(GameState& state) {
    mock_loadGameMetadata_called = true;
    return mock_loadGameMetadata_result;
}

bool MemoryCard::replayGameJournal(GameState& state) {
    mock_replayGameJournal_called = true;
    return mock_replayGameJournal_result;
}

void MemoryCard::clearActiveGame() {
    mock_clearActiveGame_called = true;
}
