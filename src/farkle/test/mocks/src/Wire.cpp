#include "Wire.h"

TwoWire Wire;
int mockWireBeginCount = 0;

void TwoWire::begin() {
    mockWireBeginCount++;
}
