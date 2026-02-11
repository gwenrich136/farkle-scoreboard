#ifndef MOCK_SERIAL_H
#define MOCK_SERIAL_H

#include <iostream>
#include <string>

class MockSerial {
public:
    void begin(int baud) {}
    void println(const char* msg) {}
    void println(int val) {}
    void print(const char* msg) {}
    void print(int val) {}
};

extern MockSerial Serial;

#endif // MOCK_SERIAL_H
