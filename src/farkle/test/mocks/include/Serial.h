#ifndef MOCK_SERIAL_H
#define MOCK_SERIAL_H

#include <iostream>
#include <string>

class MockSerial {
public:
    void begin(int baud) {}
    
    void print(const char* msg) { 
        // No-op for mock, prevents compilation error
    }
    
    void print(int val) {
        // No-op for mock
    }

    void println(const char* msg) {
        // No-op for mock
    }

    void println(int val) {
        // No-op for mock
    }
};

extern MockSerial Serial;
extern MockSerial Serial1;

#endif // MOCK_SERIAL_H
