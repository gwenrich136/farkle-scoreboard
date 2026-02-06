#include "Arduino.h"
#include <cstdlib>

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

long random(long max) {
    if (max <= 0) return 0;
    return rand() % max;
}

long random(long min, long max) {
    if (min >= max) return min;
    return min + (rand() % (max - min));
}
