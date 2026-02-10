#include <unity.h>
#include <chrono>
#include "ControlPad.h"
#include "Arduino.h"
#include <iostream>

ControlPad* controlPad;

void setUp(void) {
    controlPad = new ControlPad();
    resetMockPins();
}

void tearDown(void) {
    delete controlPad;
}

void test_benchmark_read(void) {
    // Add typical button configuration
    controlPad->addButton(2, ButtonAction::BANK);
    controlPad->addButton(5, ButtonAction::CLEAR);
    controlPad->addButton(10, ButtonAction::UP_1000);

    // Set all pins HIGH (unpressed) initially to avoid multiple press logic
    for(int i=0; i<17; i++) setMockPinState(i, HIGH);

    // Press one button
    setMockPinState(2, LOW);

    // Benchmark loop
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; i++) {
        controlPad->read();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "Benchmark read() duration: " << diff.count() << " seconds" << std::endl;
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_benchmark_read);
    return UNITY_END();
}
