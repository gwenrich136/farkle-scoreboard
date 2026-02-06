#ifndef ADAFRUIT_NEOPIXEL_H
#define ADAFRUIT_NEOPIXEL_H

#include <stdint.h>
#include <map>
#include <vector>

#define NEO_GRB 0
#define NEO_KHZ800 0
typedef uint32_t neoPixelType;

// Global state for verification
extern std::map<uint16_t, uint32_t> mockNeoPixelState;
extern int mockNeoPixelShowCount;

class Adafruit_NeoPixel {
public:
    Adafruit_NeoPixel(uint16_t n, uint8_t p, neoPixelType t) : _numPixels(n) {}

    void begin() {}

    void show() {
        mockNeoPixelShowCount++;
    }

    void setPixelColor(uint16_t n, uint32_t c) {
        if (n < _numPixels) {
            mockNeoPixelState[n] = c;
        }
    }

    void clear() {
        mockNeoPixelState.clear();
    }

    uint32_t ColorHSV(uint16_t h, uint8_t s, uint8_t v) {
        // Simple pack: 0x00HHSVVV (not realistic but sufficient for tests)
        // Hue is 0-65535, so 16 bits.
        return ((uint32_t)h << 16) | ((uint32_t)s << 8) | v;
    }

    uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
        return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }

private:
    uint16_t _numPixels;
};

#endif
