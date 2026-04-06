#include <unity.h>
#include <map>
#include <iostream>
#include <chrono>
#include "ScoreDisplay.h"
#include "Arduino.h"

extern std::map<int, std::map<int, char>> mockLedState;
extern std::map<int, int> mockLedIntensity;
extern int mockSetIntensityCount;
extern int mockClearDisplayCount;
extern int mockSetCharCount;

ScoreDisplay* display;

void setUp(void) {
    mockLedState.clear();
    mockLedIntensity.clear();
    mockSetIntensityCount = 0;
    mockClearDisplayCount = 0;
    mockSetCharCount = 0;

    // Pins don't matter for mock
    display = new ScoreDisplay(10);
    display->begin();
    display->addDisplay(ScoreDisplay::DisplayType::AT_RISK_SCORE, 0);
    display->addDisplay(ScoreDisplay::DisplayType::CURRENT_PLAYER_SCORE, 1);
    display->addDisplay(ScoreDisplay::DisplayType::COMPETITION_SCORE, 2);
}

void tearDown(void) {
    delete display;
}

/**
 * test_ScoreDisplay_Correctness_Zero
 *
 * Verifies that printing '0' results in a right-aligned '0' on the display.
 */
void test_ScoreDisplay_Correctness_Zero(void) {
    display->print_number(0, ScoreDisplay::DisplayType::AT_RISK_SCORE);
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

/**
 * test_ScoreDisplay_Correctness_Number
 *
 * Verifies that a multi-digit number is correctly formatted and right-aligned.
 */
void test_ScoreDisplay_Correctness_Number(void) {
    display->print_number(123, ScoreDisplay::DisplayType::AT_RISK_SCORE);
    // Expect "  123"
    TEST_ASSERT_EQUAL_CHAR(' ', mockLedState[0][0]);
    TEST_ASSERT_EQUAL_CHAR(' ', mockLedState[0][1]);
    TEST_ASSERT_EQUAL_CHAR('1', mockLedState[0][2]);
    TEST_ASSERT_EQUAL_CHAR('2', mockLedState[0][3]);
    TEST_ASSERT_EQUAL_CHAR('3', mockLedState[0][4]);
}

/**
 * test_ScoreDisplay_Correctness_Full
 *
 * Verifies that a 5-digit number correctly fills the entire display.
 */
void test_ScoreDisplay_Correctness_Full(void) {
    display->print_number(54321, ScoreDisplay::DisplayType::AT_RISK_SCORE);
    // Expect "54321"
    TEST_ASSERT_EQUAL_CHAR('5', mockLedState[0][0]);
    TEST_ASSERT_EQUAL_CHAR('4', mockLedState[0][1]);
    TEST_ASSERT_EQUAL_CHAR('3', mockLedState[0][2]);
    TEST_ASSERT_EQUAL_CHAR('2', mockLedState[0][3]);
    TEST_ASSERT_EQUAL_CHAR('1', mockLedState[0][4]);
}

/**
 * test_ScoreDisplay_Correctness_Overflow
 *
 * Verifies that numbers larger than 5 digits are capped at 99,999.
 */
void test_ScoreDisplay_Correctness_Overflow(void) {
    // Should cap at 99999
    display->print_number(100001, ScoreDisplay::DisplayType::AT_RISK_SCORE);
    TEST_ASSERT_EQUAL_CHAR('9', mockLedState[0][0]);
    TEST_ASSERT_EQUAL_CHAR('9', mockLedState[0][1]);
    TEST_ASSERT_EQUAL_CHAR('9', mockLedState[0][2]);
    TEST_ASSERT_EQUAL_CHAR('9', mockLedState[0][3]);
    TEST_ASSERT_EQUAL_CHAR('9', mockLedState[0][4]);
}

/**
 * test_ScoreDisplay_Blinking_Intensity
 *
 * Verifies that the blink parameter correctly toggles intensity based on simulated time.
 */
void test_ScoreDisplay_Blinking_Intensity(void) {
    // Assuming millis starts at 0
    // 0 ms -> (0 / 500) % 2 == 0 -> SCORE_BLINK_LOW (2)
    display->print_number(123, ScoreDisplay::DisplayType::AT_RISK_SCORE, true);
    TEST_ASSERT_EQUAL_INT(2, mockLedIntensity[0]);

    // Advance to 500ms
    // 500 ms -> (500 / 500) % 2 == 1 -> SCORE_BLINK_HIGH (12)
    advance_millis(500);
    display->print_number(123, ScoreDisplay::DisplayType::AT_RISK_SCORE, true);
    TEST_ASSERT_EQUAL_INT(12, mockLedIntensity[0]);

    // Advance to 1000ms
    // 1000 ms -> (1000 / 500) % 2 == 0 -> SCORE_BLINK_LOW (2)
    advance_millis(500);
    display->print_number(123, ScoreDisplay::DisplayType::AT_RISK_SCORE, true);
    TEST_ASSERT_EQUAL_INT(2, mockLedIntensity[0]);

    // Turn off blinking
    display->print_number(123, ScoreDisplay::DisplayType::AT_RISK_SCORE, false);
    TEST_ASSERT_EQUAL_INT(8, mockLedIntensity[0]);
}

/**
 * test_ScoreDisplay_Clear
 *
 * Verifies that clearing the display sets all digits to empty spaces.
 */
void test_ScoreDisplay_Clear(void) {
    display->print_number(123, ScoreDisplay::DisplayType::AT_RISK_SCORE);
    TEST_ASSERT_EQUAL_CHAR('3', mockLedState[0][4]);

    display->clear(ScoreDisplay::DisplayType::AT_RISK_SCORE);
    // clearDisplay should result in all spaces or similar in the mock
    // In our mock LedControl, clearDisplay sets all digits to ' '
    TEST_ASSERT_EQUAL_CHAR(' ', mockLedState[0][0]);
    TEST_ASSERT_EQUAL_CHAR(' ', mockLedState[0][1]);
    TEST_ASSERT_EQUAL_CHAR(' ', mockLedState[0][2]);
    TEST_ASSERT_EQUAL_CHAR(' ', mockLedState[0][3]);
    TEST_ASSERT_EQUAL_CHAR(' ', mockLedState[0][4]);
}

/**
 * test_ScoreDisplay_HardwareInteractionOptimization
 *
 * This test verifies that the ScoreDisplay maintains an internal state
 * and skips redundant calls to the LedControl hardware library when
 * the printed content or display intensity hasn't changed.
 */
void test_ScoreDisplay_HardwareInteractionOptimization(void) {
    // Reset counters after begin()
    mockSetIntensityCount = 0;
    mockClearDisplayCount = 0;
    mockSetCharCount = 0;

    // First call should trigger hardware updates
    display->print_number(123, ScoreDisplay::DisplayType::AT_RISK_SCORE);
    TEST_ASSERT_TRUE(mockSetCharCount > 0);
    int initialSetCharCount = mockSetCharCount;
    int initialSetIntensityCount = mockSetIntensityCount;

    // Second call with same parameters should NOT trigger hardware updates
    display->print_number(123, ScoreDisplay::DisplayType::AT_RISK_SCORE);
    TEST_ASSERT_EQUAL_INT(initialSetCharCount, mockSetCharCount);
    TEST_ASSERT_EQUAL_INT(initialSetIntensityCount, mockSetIntensityCount);

    // Call with different number should trigger updates
    display->print_number(456, ScoreDisplay::DisplayType::AT_RISK_SCORE);
    TEST_ASSERT_TRUE(mockSetCharCount > initialSetCharCount);
    initialSetCharCount = mockSetCharCount;

    // Call with same number but DIFFERENT blink mode should trigger intensity update
    // Note: at 0ms, blink=true intensity (2) is different from default (8)
    display->print_number(456, ScoreDisplay::DisplayType::AT_RISK_SCORE, true);
    TEST_ASSERT_TRUE(mockSetIntensityCount > initialSetIntensityCount);
    initialSetIntensityCount = mockSetIntensityCount;

    // Subsequent call with same number and same blink mode (and same millis window)
    // should NOT trigger updates
    display->print_number(456, ScoreDisplay::DisplayType::AT_RISK_SCORE, true);
    TEST_ASSERT_EQUAL_INT(initialSetCharCount, mockSetCharCount);
    TEST_ASSERT_EQUAL_INT(initialSetIntensityCount, mockSetIntensityCount);

    // Test clear optimization
    mockClearDisplayCount = 0;
    display->clear(ScoreDisplay::DisplayType::AT_RISK_SCORE);
    TEST_ASSERT_TRUE(mockClearDisplayCount > 0);
    int clearCount = mockClearDisplayCount;
    display->clear(ScoreDisplay::DisplayType::AT_RISK_SCORE);
    TEST_ASSERT_EQUAL_INT(clearCount, mockClearDisplayCount); // Should not increase
}

void test_ScoreDisplay_Performance(void) {
    const int iterations = 10000;
    auto start = std::chrono::high_resolution_clock::now();

    // We mix some calls to prevent compiler from optimizing too much if it could,
    // though for side-effects it shouldn't.
    for (int i = 0; i < iterations; i++) {
        display->print_number(12345, ScoreDisplay::DisplayType::AT_RISK_SCORE);
        display->print_number(123, ScoreDisplay::DisplayType::AT_RISK_SCORE);
        display->print_number(0, ScoreDisplay::DisplayType::AT_RISK_SCORE);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "PERFORMANCE_RESULT: " << duration.count() << " microseconds for " << (iterations * 3) << " calls" << std::endl;
}

/**
 * test_ScoreDisplay_Security_InvalidType
 *
 * Verifies that using an invalid DisplayType does not cause a crash
 * and handles the error gracefully (logging assertion failure).
 */
void test_ScoreDisplay_Security_InvalidType(void) {
    ScoreDisplay::DisplayType invalidType = static_cast<ScoreDisplay::DisplayType>(99);

    // These should not crash
    display->addDisplay(invalidType, 99);
    display->print_number(123, invalidType);
    display->clear(invalidType);

    // We can't easily verify the Serial output in this environment without modifying mocks,
    // but successful execution of this test confirms no crash occurred.
    TEST_ASSERT_TRUE(true);
}

/**
 * test_ScoreDisplay_Security_NegativeOverflow
 *
 * Verifies that negative numbers with more than 5 characters (e.g. -12345)
 * do not cause out-of-bounds array access and are clamped to a displayable minimum (-9999).
 */
void test_ScoreDisplay_Security_NegativeOverflow(void) {
    // -12345 has 6 characters including sign.
    // If not clamped, len=6. emptySlots = 5 - 6 = -1.
    // Loop 2: i=0 -> index = 0 + (-1) = -1. OOB access!

    // We use AT_RISK_SCORE (device 0)
    display->print_number(-12345, ScoreDisplay::DisplayType::AT_RISK_SCORE);

    bool hasNegativeIndex = false;
    for (auto const& [digit, val] : mockLedState[0]) {
        // MAX7219 supports 0-7. ScoreDisplay uses 0-4.
        // LedControl mock initializes 0-7 to ' '.
        // We only care about negative indices which indicate the specific vulnerability.
        if (digit < 0) {
            hasNegativeIndex = true;
        }
    }

    TEST_ASSERT_FALSE_MESSAGE(hasNegativeIndex, "Out of bounds access detected! ScoreDisplay wrote to invalid digit index.");

    // Verify correct clamping to -9999
    // Expected: "-9999"
    TEST_ASSERT_EQUAL_CHAR('-', mockLedState[0][0]);
    TEST_ASSERT_EQUAL_CHAR('9', mockLedState[0][1]);
    TEST_ASSERT_EQUAL_CHAR('9', mockLedState[0][2]);
    TEST_ASSERT_EQUAL_CHAR('9', mockLedState[0][3]);
    TEST_ASSERT_EQUAL_CHAR('9', mockLedState[0][4]);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_ScoreDisplay_Correctness_Zero);
    RUN_TEST(test_ScoreDisplay_Correctness_Number);
    RUN_TEST(test_ScoreDisplay_Correctness_Full);
    RUN_TEST(test_ScoreDisplay_Correctness_Overflow);
    RUN_TEST(test_ScoreDisplay_Blinking_Intensity);
    RUN_TEST(test_ScoreDisplay_Clear);
    RUN_TEST(test_ScoreDisplay_HardwareInteractionOptimization);
    RUN_TEST(test_ScoreDisplay_Performance);
    RUN_TEST(test_ScoreDisplay_Security_InvalidType);
    RUN_TEST(test_ScoreDisplay_Security_NegativeOverflow);
    return UNITY_END();
}
