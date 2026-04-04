#ifndef MD_MAX72XX_H
#define MD_MAX72XX_H

#include <stdint.h>
#include <map>
#include <string>

extern std::map<int, std::map<int, char>> mockLedState;
extern std::map<int, int> mockLedIntensity;

extern int mockSetIntensityCount;
extern int mockClearDisplayCount;
extern int mockSetCharCount;

class MD_MAX72XX {
public:
    enum moduleType_t { PAROLA_HW, GENERIC_HW, ICSTATION_HW, FC16_HW };
    enum controlRequest_t { INTENSITY, TEST, SCANLIMIT, SHUTDOWN, UPDATE };

    MD_MAX72XX(moduleType_t mod, int csPin, int numDevices) {
    }

    void begin() {}

    void control(int addr, controlRequest_t mode, int value) {
        if (mode == INTENSITY) {
            mockSetIntensityCount++;
            mockLedIntensity[addr] = value;
        }
    }

    void clear(int addr) {
        mockClearDisplayCount++;
        mockLedState[addr].clear();
        for (int i = 0; i < 8; i++) {
            mockLedState[addr][i] = ' ';
        }
    }

    void clear() {
        // clear all
    }

    void setColumn(int col, uint8_t value) {
        mockSetCharCount++;
        int addr = col / 8;
        int digit = col % 8;

        // Reverse translation for test verification
        char c = ' ';
        switch(value) {
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
        mockLedState[addr][digit] = c;
    }
};

#endif
