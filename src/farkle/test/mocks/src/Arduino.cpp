#include "Arduino.h"

static unsigned long mocked_millis = 0;

unsigned long millis() {
    return mocked_millis;
}

void delay(unsigned long ms) {
    mocked_millis += ms;
}

void advance_millis(unsigned long ms) {
    mocked_millis += ms;
}
