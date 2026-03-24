#ifndef _ADAFRUIT_ST7789_H_
#define _ADAFRUIT_ST7789_H_

#include "Adafruit_GFX.h"

// Hardware SPI
#define SPI_DEFAULT_FREQ 32000000

class Adafruit_ST7789 : public Adafruit_GFX {
public:
    Adafruit_ST7789(int8_t cs, int8_t dc, int8_t rst)
        : Adafruit_GFX(240, 240), _cs(cs), _dc(dc), _rst(rst) {}

    void init(uint16_t width, uint16_t height, uint8_t spiMode = 0) {
        mockAdafruitInitCount++;
        _width = width;
        _height = height;
    }

private:
    int8_t _cs, _dc, _rst;
};

#endif // _ADAFRUIT_ST7789_H_
