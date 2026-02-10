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

    // Should still return BANK (first action)
    TEST_ASSERT_EQUAL(ButtonAction::BANK, controlPad->read());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_add_none_action_fails);
    RUN_TEST(test_add_duplicate_pin_fails);
    return UNITY_END();
}
