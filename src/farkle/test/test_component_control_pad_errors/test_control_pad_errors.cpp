#include <unity.h>
#include <setjmp.h>
#include <signal.h>
#include <cstdlib>

// Define a jump buffer to handle assertion failure
static jmp_buf assert_jump_buf;
static bool expect_assert = false;

// Custom assertion handler to replace standard assert
// We need to define this BEFORE including ControlPad.h if we can macro-replace assert
// However, assert is usually a macro. If we want to intercept it, we might need to redefine it globally
// or link against a custom __assert_fail.

// Since we are compiling C++, and assert is a macro, we can try to undef and redefine it.
// But ControlPad.cpp includes <cassert>. We can't easily modify ControlPad.cpp's inclusion without changing the file.
// Wait, ControlPad.cpp is compiled separately. We can't change how it compiles from here.
// But we are in a unit test environment.

// In `component_tests` environment, `ControlPad.cpp` is compiled and linked.
// If we want to catch `assert(false)` inside `ControlPad.cpp`, we need to mock `assert`.
// `assert` typically calls `__assert_fail` or `__assert_func` on POSIX/glibc.
// We can try to override `__assert_fail` if the linker allows weak symbols or if we are lucky.

// Let's try to override __assert_fail for Linux/glibc environment (native platformio)
extern "C" void __assert_fail(const char *assertion, const char *file, unsigned int line, const char *function) {
    if (expect_assert) {
        longjmp(assert_jump_buf, 1);
    } else {
        // Fallback to abort if we weren't expecting it
        abort();
    }
}

// Also handle the macos version just in case (though we are likely on linux)
extern "C" void __assert_rtn(const char *func, const char *file, int line, const char *condition) {
     if (expect_assert) {
        longjmp(assert_jump_buf, 1);
    } else {
        abort();
    }
}

#include "ControlPad.h"
#include "Arduino.h"
#include "ButtonActions.h"

ControlPad* controlPad;

void setUp(void) {
    controlPad = new ControlPad();
    resetMockPins();
    expect_assert = false;
}

void tearDown(void) {
    delete controlPad;
}

void test_add_none_action_fails(void) {
    int pin = 2;
    expect_assert = true;

    if (setjmp(assert_jump_buf) == 0) {
        // Attempt to add a button with NONE action
        controlPad->addButton(pin, ButtonAction::NONE);
        TEST_FAIL_MESSAGE("Should have asserted");
    } else {
        // Assertion caught! Pass.
        expect_assert = false;
    }

    // Verify nothing happened (pinMode not called)
    TEST_ASSERT_EQUAL(-1, getMockPinMode(pin));
}

void test_add_duplicate_pin_fails(void) {
    int pin = 3;
    controlPad->addButton(pin, ButtonAction::BANK);
    TEST_ASSERT_EQUAL(INPUT_PULLUP, getMockPinMode(pin));

    expect_assert = true;
    if (setjmp(assert_jump_buf) == 0) {
        // Attempt to overwrite with CLEAR
        controlPad->addButton(pin, ButtonAction::CLEAR);
        TEST_FAIL_MESSAGE("Should have asserted");
    } else {
        // Assertion caught! Pass.
        expect_assert = false;
    }

    // Should still return BANK (first action)
    setMockPinState(pin, LOW);
    TEST_ASSERT_EQUAL(ButtonAction::BANK, controlPad->read());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_add_none_action_fails);
    RUN_TEST(test_add_duplicate_pin_fails);
    return UNITY_END();
}
