#include <unity.h>
#include "ControlPad.h"
#include "Arduino.h"
#include "Input.h"

ControlPad* controlPad;

void setMockBusState(uint8_t state) {
    // Active low logic: 1 means LOW
    setMockPinState(CONTROL_PAD_BUS_0_PIN, (state & 0b001) ? LOW : HIGH);
    setMockPinState(CONTROL_PAD_BUS_1_PIN, (state & 0b010) ? LOW : HIGH);
    setMockPinState(CONTROL_PAD_BUS_2_PIN, (state & 0b100) ? LOW : HIGH);
}

void setUp(void) {
    resetMockPins();
    // Set default bus state to NONE (000, all HIGH)
    setMockBusState(0);
    setMockPinState(SELECT_PIN, HIGH);

    controlPad = new ControlPad();
    controlPad->begin(); // Added to initialize hardware/interrupts

    // Initial reads to set up steady states
    controlPad->read();
    advance_millis(DEBOUNCE_DELAY + 10);
    controlPad->read();
}

void tearDown(void) {
    delete controlPad;
}

void test_bus_stability_check(void) {
    // 1. Set Bus to 1 (CLEAR)
    setMockBusState(1);

    // 2. Read immediately -> Should be NONE (unstable)
    GameInput input = controlPad->read();
    TEST_ASSERT_EQUAL(ButtonAction::NONE, input.action);

    // 3. Advance time by 49ms -> Still NONE
    advance_millis(49);
    input = controlPad->read();
    TEST_ASSERT_EQUAL(ButtonAction::NONE, input.action);

    // 4. Advance time by 2ms (total 51ms) -> Should be CLEAR
    advance_millis(2);
    input = controlPad->read();
    TEST_ASSERT_EQUAL(ButtonAction::CLEAR, input.action);

    // 5. Change Bus to 2 (FARKLE)
    setMockBusState(2);

    // 6. Read immediately -> Should be NONE (unstable)
    input = controlPad->read();
    TEST_ASSERT_EQUAL(ButtonAction::NONE, input.action);

    // 7. Advance 51ms -> FARKLE
    advance_millis(51);
    input = controlPad->read();
    TEST_ASSERT_EQUAL(ButtonAction::FARKLE, input.action);
}

void test_ghost_undo(void) {
    // Bank + Clear = UNDO (101 binary, decimal 5)
    setMockBusState(5);

    controlPad->read();
    advance_millis(DEBOUNCE_DELAY + 10);

    GameInput input = controlPad->read();
    TEST_ASSERT_EQUAL(ButtonAction::UNDO, input.action);
}

void test_input_priority(void) {
    // SELECT button active (LOW)
    setMockPinState(SELECT_PIN, LOW);

    // Set a bus action as well
    setMockBusState(4); // BANK

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

    // Priority: SELECT takes precedence over BUS according to original logic map
    // Note: If you want BUS to take priority, you should adjust checkSelectInput / read order.
    // Currently, SELECT comes first.
    TEST_ASSERT_EQUAL(ButtonAction::SELECT, input.action);
    // Rotation suppressed (should be 0 because a button took priority)
    TEST_ASSERT_EQUAL(0, input.rotationDelta);
}

void test_no_auto_repeat(void) {
    // Press BANK (bus state 4)
    setMockBusState(4);
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
    setMockBusState(0);
    // Debounce release
    controlPad->read();
    advance_millis(DEBOUNCE_DELAY + 1);
    controlPad->read(); // Process release state

    // Press again
    setMockBusState(4);
    controlPad->read();
    advance_millis(DEBOUNCE_DELAY + 1);
    input = controlPad->read();
    TEST_ASSERT_EQUAL(ButtonAction::BANK, input.action);
}

void test_encoder_accumulation(void) {
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
    RUN_TEST(test_bus_stability_check);
    RUN_TEST(test_ghost_undo);
    RUN_TEST(test_input_priority);
    RUN_TEST(test_no_auto_repeat);
    RUN_TEST(test_encoder_accumulation);
    return UNITY_END();
}
