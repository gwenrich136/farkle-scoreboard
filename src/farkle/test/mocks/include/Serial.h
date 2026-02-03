#ifndef MOCK_SERIAL_H
#define MOCK_SERIAL_H

#include <iostream>

class MockSerial {
public:
    void begin(int baud) {}
    void println(const char* msg) {}
};

extern MockSerial Serial;

#endif // MOCK_SERIAL_H
