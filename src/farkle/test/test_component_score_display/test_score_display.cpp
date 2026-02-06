#include <unity.h>
#include <map>
#include <iostream>
#include <chrono>
#include "ScoreDisplay.h"
#include "Arduino.h"

extern std::map<int, std::map<int, char>> mockLedState;
extern std::map<int, int> mockLedIntensity;

ScoreDisplay* display;

void setUp(void) {
    mockLedState.clear();
    mockLedIntensity.clear();
    // Pins don't matter for mock
    display = new ScoreDisplay(10, 11, 12);
    display->begin();
}

void tearDown(void) {
    delete display;
}

void test_ScoreDisplay_Correctness_Zero(void) {
    display->print_number(0, 0);
    // Expect "    0" (4 spaces, 1 zero)
    // Note: implementation depends on how many digits are flushed.
    // print_number fills 'emptySlots' with ' ', then the number.
    // It assumes NUM_DIGITS_PER_DISPLAY = 5.

    TEST_ASSERT_EQUAL_CHAR(' ', mockLedState[0][0]);
    TEST_ASSERT_EQUAL_CHAR(' ', mockLedState[0][1]);
    TEST_ASSERT_EQUAL_CHAR(' ', mockLedState[0][2]);
    TEST_ASSERT_EQUAL_CHAR(' ', mockLedState[0][3]);
    TEST_ASSERT_EQUAL_CHAR('0', mockLedState[0][4]);
}

void test_ScoreDisplay_Correctness_Number(void) {
    display->print_number(123, 0);
    // Expect "  123"
    TEST_ASSERT_EQUAL_CHAR(' ', mockLedState[0][0]);
    TEST_ASSERT_EQUAL_CHAR(' ', mockLedState[0][1]);
    TEST_ASSERT_EQUAL_CHAR('1', mockLedState[0][2]);
    TEST_ASSERT_EQUAL_CHAR('2', mockLedState[0][3]);
    TEST_ASSERT_EQUAL_CHAR('3', mockLedState[0][4]);
}

void test_ScoreDisplay_Correctness_Full(void) {
    display->print_number(54321, 0);
    // Expect "54321"
    TEST_ASSERT_EQUAL_CHAR('5', mockLedState[0][0]);
    TEST_ASSERT_EQUAL_CHAR('4', mockLedState[0][1]);
    TEST_ASSERT_EQUAL_CHAR('3', mockLedState[0][2]);
    TEST_ASSERT_EQUAL_CHAR('2', mockLedState[0][3]);
    TEST_ASSERT_EQUAL_CHAR('1', mockLedState[0][4]);
}

void test_ScoreDisplay_Correctness_Overflow(void) {
    // Should cap at 99999
    display->print_number(100001, 0);
    TEST_ASSERT_EQUAL_CHAR('9', mockLedState[0][0]);
    TEST_ASSERT_EQUAL_CHAR('9', mockLedState[0][1]);
    TEST_ASSERT_EQUAL_CHAR('9', mockLedState[0][2]);
    TEST_ASSERT_EQUAL_CHAR('9', mockLedState[0][3]);
    TEST_ASSERT_EQUAL_CHAR('9', mockLedState[0][4]);
}

void test_ScoreDisplay_Blinking_Intensity(void) {
    // Assuming millis starts at 0
    // 0 ms -> (0 / 500) % 2 == 0 -> SCORE_BLINK_LOW (2)
    display->print_number(123, 0, true);
    TEST_ASSERT_EQUAL_INT(2, mockLedIntensity[0]);

    // Advance to 500ms
    // 500 ms -> (500 / 500) % 2 == 1 -> SCORE_BLINK_HIGH (12)
    advance_millis(500);
    display->print_number(123, 0, true);
    TEST_ASSERT_EQUAL_INT(12, mockLedIntensity[0]);

    // Advance to 1000ms
    // 1000 ms -> (1000 / 500) % 2 == 0 -> SCORE_BLINK_LOW (2)
    advance_millis(500);
    display->print_number(123, 0, true);
    TEST_ASSERT_EQUAL_INT(2, mockLedIntensity[0]);

    // Turn off blinking
    display->print_number(123, 0, false);
    TEST_ASSERT_EQUAL_INT(8, mockLedIntensity[0]);
}

void test_ScoreDisplay_Performance(void) {
    const int iterations = 10000;
    auto start = std::chrono::high_resolution_clock::now();

    // We mix some calls to prevent compiler from optimizing too much if it could,
    // though for side-effects it shouldn't.
    for (int i = 0; i < iterations; i++) {
        display->print_number(12345, 0);
        display->print_number(123, 0);
        display->print_number(0, 0);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "PERFORMANCE_RESULT: " << duration.count() << " microseconds for " << (iterations * 3) << " calls" << std::endl;
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_ScoreDisplay_Correctness_Zero);
    RUN_TEST(test_ScoreDisplay_Correctness_Number);
    RUN_TEST(test_ScoreDisplay_Correctness_Full);
    RUN_TEST(test_ScoreDisplay_Correctness_Overflow);
    RUN_TEST(test_ScoreDisplay_Blinking_Intensity);
    RUN_TEST(test_ScoreDisplay_Performance);
    return UNITY_END();
}
