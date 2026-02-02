#include "FarkleWarningLights.h"

FarkleWarningLights::FarkleWarningLights(int yellowPin, int redPin) {
    // Constructor can be empty for the mock
}

void FarkleWarningLights::begin() {
    // Begin can be empty for the mock
}

void FarkleWarningLights::farkle_state(int state) {
    captured_state = state;
}

void FarkleWarningLights::alternate() {
    // Empty mock implementation
}
