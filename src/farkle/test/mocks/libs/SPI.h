#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include <stdint.h>
#include <map>

// Test tracking variables
extern std::map<int, std::map<int, char>> mockLedState;
extern std::map<int, int> mockLedIntensity;
extern int mockSetIntensityCount;
extern int mockClearDisplayCount;
extern int mockSetCharCount;

#define MSBFIRST 1
#define SPI_MODE0 0

class SPISettings {
public:
    SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {}
};

class SPIClass {
private:
    uint8_t _currentReg;
    uint8_t _currentData;
    bool _isReg;

    // The ScoreDisplay writes NOOP (0x00, 0x00) to devices we don't care about,
    // and (reg, data) to the target device. Since it's a daisy chain, the FIRST
    // bytes sent end up in the LAST device.
    // In ScoreDisplay::max7219_write:
    // for (int i = NUM_DEVICES - 1; i >= 0; i--) { ... }
    // This perfectly matches the physical shift register. We need to track which
    // device receives the actual command.

    int _shiftPosition;
    int _targetDevice;
    uint8_t _targetReg;
    uint8_t _targetData;

public:
    void begin() {}
    void end() {}

    void beginTransaction(SPISettings settings) {
        _shiftPosition = 2; // NUM_DEVICES - 1
        _isReg = true;
        _targetDevice = -1;
    }

    void endTransaction() {
        if (_targetDevice >= 0) {
            handleCommand(_targetDevice, _targetReg, _targetData);
        }
    }

    void transfer(uint8_t data) {
        if (_isReg) {
            _currentReg = data;
            _isReg = false;
        } else {
            _currentData = data;
            _isReg = true;

            // If it's not a NOOP, it's a real command for the device at the current shift position
            if (_currentReg != 0x00) {
                _targetDevice = _shiftPosition;
                _targetReg = _currentReg;
                _targetData = _currentData;
            }

            _shiftPosition--;
        }
    }

private:
    void handleCommand(int device, uint8_t reg, uint8_t data) {
        if (reg >= 0x01 && reg <= 0x08) {
            // Digit register
            int digit = reg - 0x01; // 0-indexed internally

            char c = ' ';
            switch(data) {
                case 0x7E: c = '0'; break;
                case 0x30: c = '1'; break;
                case 0x6D: c = '2'; break;
                case 0x79: c = '3'; break;
                case 0x33: c = '4'; break;
                case 0x5B: c = '5'; break;
                case 0x5F: c = '6'; break;
                case 0x70: c = '7'; break;
                case 0x7F: c = '8'; break;
                case 0x7B: c = '9'; break;
                case 0x01: c = '-'; break;
                case 0x00: c = ' '; break;
            }
            mockLedState[device][digit] = c;

            if (data == 0x00) {
                mockClearDisplayCount++; // Heuristic for test compatibility
            } else {
                mockSetCharCount++;
            }
        } else if (reg == 0x0A) {
            // Intensity register
            mockLedIntensity[device] = data;
            mockSetIntensityCount++;
        }
    }
};

extern SPIClass SPI;

#endif
