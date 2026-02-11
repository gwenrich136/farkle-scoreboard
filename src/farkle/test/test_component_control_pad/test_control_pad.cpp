#include <unity.h>
#include "ControlPad.h"
#include "Arduino.h"
#include "ButtonActions.h"

ControlPad* controlPad;

void setUp(void) {
    controlPad = new ControlPad();
    resetMockPins();
}

void tearDown(void) {
    delete controlPad;
}

void test_add_valid_button(void) {
    int pin = 2;
    ButtonAction action = ButtonAction::BANK;

    controlPad->addButton(pin, action);

    // Verify pinMode was called with INPUT_PULLUP
    TEST_ASSERT_EQUAL_INT(INPUT_PULLUP, getMockPinMode(pin));
}

void test_add_invalid_pin_negative(void) {
    int pin = -1;
    ButtonAction action = ButtonAction::BANK;

    controlPad->addButton(pin, action);

    // Verify pinMode was NOT called for this pin
    TEST_ASSERT_EQUAL_INT(-1, getMockPinMode(pin));
}

void test_add_invalid_pin_too_large(void) {
    int pin = 17; // MAX_PINS is 17, so 17 is OOB (0-16 are valid if MAX_PINS was size, but code says MAX_PINS is 17 and array is size 17, so indices 0..16)
    // Wait, let me check ControlPad.h
    // #define MAX_PINS 17
    // ButtonAction _buttonMap[MAX_PINS];
    // So indices 0 to 16 are valid. 17 is OOB.

    ButtonAction action = ButtonAction::BANK;

    controlPad->addButton(pin, action);

    // Verify pinMode was NOT called for this pin
    TEST_ASSERT_EQUAL_INT(-1, getMockPinMode(pin));
}

void test_read_valid_button(void) {
    int pin = 5;
    ButtonAction action = ButtonAction::CLEAR;

    controlPad->addButton(pin, action);

    // Simulate button press (LOW)
    setMockPinState(pin, LOW);
    controlPad->read(); // Start debounce
    advance_millis(DEBOUNCE_DELAY + 1);

    TEST_ASSERT_EQUAL(action, controlPad->read());
}

void test_add_none_action_fails(void) {
    int pin = 2;
    // Attempt to add a button with NONE action
    controlPad->addButton(pin, ButtonAction::NONE);

    // Verify pinMode was not called (remains default -1)
    TEST_ASSERT_EQUAL(-1, getMockPinMode(pin));

    // Press the button
    setMockPinState(pin, LOW);

    // Should read NONE because it was not added
    TEST_ASSERT_EQUAL(ButtonAction::NONE, controlPad->read());
}

void test_add_duplicate_pin_fails(void) {
    int pin = 3;
    controlPad->addButton(pin, ButtonAction::BANK);

    // Verify first add worked
    TEST_ASSERT_EQUAL(INPUT_PULLUP, getMockPinMode(pin));

    // Attempt to overwrite with CLEAR
    controlPad->addButton(pin, ButtonAction::CLEAR);

    // Press the button
    setMockPinState(pin, LOW);
    controlPad->read(); // Start debounce
    advance_millis(DEBOUNCE_DELAY + 1);

    // Should still return BANK (first action)
    TEST_ASSERT_EQUAL(ButtonAction::BANK, controlPad->read());
}

void test_read_with_bouncing(void) {
    int pin = 5;
    ButtonAction action = ButtonAction::CLEAR;
    controlPad->addButton(pin, action);

    // 1. Initial press (LOW)
    setMockPinState(pin, LOW);
    controlPad->read(); // Trigger debounce timer
    advance_millis(DEBOUNCE_DELAY + 1);
    TEST_ASSERT_EQUAL(action, controlPad->read());

    // 2. Bounce to HIGH (released) briefly
    setMockPinState(pin, HIGH);
    controlPad->read(); // Should still be debounced as LOW
    TEST_ASSERT_EQUAL(ButtonAction::NONE, controlPad->read()); // NONE because pressedAction (action) == _lastAction (action)

    // 3. Bounce back to LOW (pressed)
    setMockPinState(pin, LOW);
    // Should still return NONE because it never stabilized at HIGH
    TEST_ASSERT_EQUAL(ButtonAction::NONE, controlPad->read());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_add_valid_button);
    RUN_TEST(test_add_invalid_pin_negative);
    RUN_TEST(test_add_invalid_pin_too_large);
    RUN_TEST(test_read_valid_button);
    RUN_TEST(test_add_none_action_fails);
    RUN_TEST(test_add_duplicate_pin_fails);
    RUN_TEST(test_read_with_bouncing);
    return UNITY_END();
}
