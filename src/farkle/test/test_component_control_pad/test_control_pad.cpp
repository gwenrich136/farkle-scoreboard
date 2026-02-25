#include <unity.h>
#include "ControlPad.h"
#include "Arduino.h"
#include "Input.h"

ControlPad* controlPad;

void setUp(void) {
    resetMockPins();
    // Set ADC to a value that maps to NONE (>= 700) so it doesn't interfere with digital/encoder tests
    setMockAnalogPin(ADC_PIN, 1000);
    controlPad = new ControlPad();
    // Stabilize the ADC at 1000 (NONE)
    controlPad->read(); // Initial read sets _lastAdcValue
    advance_millis(ADC_STABILITY_THRESHOLD_MS + 10);
    controlPad->read(); // Should be stable NONE
}

void tearDown(void) {
    delete controlPad;
}

void test_adc_stability_check(void) {
    // 0 (CLEAR), 93 (+50), 328 (+100), 512 (+500)

    // 1. Set ADC to 93 (+50)
    setMockAnalogPin(ADC_PIN, 93);

    // 2. Read immediately -> Should be NONE (unstable)
    // Previous value was 1000. New is 93. Change detected.
    GameInput input = controlPad->read();
    TEST_ASSERT_EQUAL(ButtonAction::NONE, input.action);

    // 3. Advance time by 49ms -> Still NONE
    advance_millis(49);
    input = controlPad->read();
    TEST_ASSERT_EQUAL(ButtonAction::NONE, input.action);

    // 4. Advance time by 2ms (total 51ms) -> Should be PLUS_50
    advance_millis(2);
    input = controlPad->read();
    TEST_ASSERT_EQUAL(ButtonAction::PLUS_50, input.action);

    // 5. Change ADC to 328 (+100)
    setMockAnalogPin(ADC_PIN, 328);

    // 6. Read immediately -> Should be NONE (unstable)
    input = controlPad->read();
    TEST_ASSERT_EQUAL(ButtonAction::NONE, input.action);

    // 7. Advance 51ms -> PLUS_100
    advance_millis(51);
    input = controlPad->read();
    TEST_ASSERT_EQUAL(ButtonAction::PLUS_100, input.action);
}

void test_input_priority(void) {
    // BANK_PIN (6) active (LOW)
    setMockPinState(BANK_PIN, LOW);

    // Trigger debounce logic
    controlPad->read();

    // Also trigger encoder interrupts to generate delta
    // Simulating a CW tick
    setMockPinState(ENCODER_PIN_A, LOW);
    triggerInterrupt(ENCODER_PIN_A);
    setMockPinState(ENCODER_PIN_B, LOW);
    triggerInterrupt(ENCODER_PIN_B);
    setMockPinState(ENCODER_PIN_A, HIGH);
    triggerInterrupt(ENCODER_PIN_A);
    setMockPinState(ENCODER_PIN_B, HIGH);
    triggerInterrupt(ENCODER_PIN_B);

    // Advance time for debounce of digital pin
    advance_millis(DEBOUNCE_DELAY + 1);

    GameInput input = controlPad->read();

    // Priority: BANK
    TEST_ASSERT_EQUAL(ButtonAction::BANK, input.action);
    // Rotation suppressed (should be 0 because BANK took priority)
    TEST_ASSERT_EQUAL(0, input.rotationDelta);
}

void test_no_auto_repeat(void) {
    // Press BANK
    setMockPinState(BANK_PIN, LOW);
    // Debounce
    controlPad->read();
    advance_millis(DEBOUNCE_DELAY + 1);

    // First read: BANK
    GameInput input = controlPad->read();
    TEST_ASSERT_EQUAL(ButtonAction::BANK, input.action);

    // Second read: NONE (still held)
    input = controlPad->read();
    TEST_ASSERT_EQUAL(ButtonAction::NONE, input.action);

    // Release
    setMockPinState(BANK_PIN, HIGH);
    // Debounce release
    controlPad->read();
    advance_millis(DEBOUNCE_DELAY + 1);
    controlPad->read(); // Process release state

    // Press again
    setMockPinState(BANK_PIN, LOW);
    controlPad->read();
    advance_millis(DEBOUNCE_DELAY + 1);
    input = controlPad->read();
    TEST_ASSERT_EQUAL(ButtonAction::BANK, input.action);
}

void test_encoder_accumulation(void) {
    // ADC should be NONE (1000) from setUp

    // Simulate multiple CW ticks
    // Tick 1
    setMockPinState(ENCODER_PIN_A, LOW); triggerInterrupt(ENCODER_PIN_A);
    setMockPinState(ENCODER_PIN_B, LOW); triggerInterrupt(ENCODER_PIN_B);
    setMockPinState(ENCODER_PIN_A, HIGH); triggerInterrupt(ENCODER_PIN_A);
    setMockPinState(ENCODER_PIN_B, HIGH); triggerInterrupt(ENCODER_PIN_B);

    // Tick 2
    setMockPinState(ENCODER_PIN_A, LOW); triggerInterrupt(ENCODER_PIN_A);
    setMockPinState(ENCODER_PIN_B, LOW); triggerInterrupt(ENCODER_PIN_B);
    setMockPinState(ENCODER_PIN_A, HIGH); triggerInterrupt(ENCODER_PIN_A);
    setMockPinState(ENCODER_PIN_B, HIGH); triggerInterrupt(ENCODER_PIN_B);

    GameInput input = controlPad->read();

    // Should have accumulated delta (e.g. 2 or -2 depending on direction logic)
    // We just check it's not 0 and magnitude > 0
    TEST_ASSERT_NOT_EQUAL(0, input.rotationDelta);

    // Reset check
    input = controlPad->read();
    TEST_ASSERT_EQUAL(0, input.rotationDelta);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_adc_stability_check);
    RUN_TEST(test_input_priority);
    RUN_TEST(test_no_auto_repeat);
    RUN_TEST(test_encoder_accumulation);
    return UNITY_END();
}
