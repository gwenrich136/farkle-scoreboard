#include <unity.h>
#include "TextDisplayV2.h"
#include "Adafruit_ST7789.h"
#include "Adafruit_GFX.h"

// Access to mock variables from Adafruit_GFX.cpp
extern std::vector<MockAdafruitPrintCall> mockAdafruitPrintCalls;
extern std::vector<MockAdafruitDrawLineCall> mockAdafruitDrawLineCalls;
extern std::vector<MockAdafruitFillRectCall> mockAdafruitFillRectCalls;
extern int mockAdafruitInitCount;
extern int mockAdafruitSetRotationCount;
extern int mockAdafruitFillScreenCount;

TextDisplayV2* textDisplayV2 = nullptr;

void setUp(void) {
    mockAdafruitPrintCalls.clear();
    mockAdafruitDrawLineCalls.clear();
    mockAdafruitFillRectCalls.clear();
    mockAdafruitInitCount = 0;
    mockAdafruitSetRotationCount = 0;
    mockAdafruitFillScreenCount = 0;

    if (textDisplayV2) delete textDisplayV2;
    // A4=A4, A5=A5, D7=7, D8=8
    textDisplayV2 = new TextDisplayV2(A4, A5, 7, 8);
}

void tearDown(void) {
    if (textDisplayV2) {
        delete textDisplayV2;
        textDisplayV2 = nullptr;
    }
}

void test_begin(void) {
    textDisplayV2->begin();
    TEST_ASSERT_EQUAL(1, mockAdafruitInitCount);
    TEST_ASSERT_EQUAL(1, mockAdafruitSetRotationCount);
    TEST_ASSERT_EQUAL(1, mockAdafruitFillScreenCount); // From the initial fillScreen
}

void test_print(void) {
    textDisplayV2->print("Hello V2");

    // 1 call to print
    TEST_ASSERT_EQUAL(1, mockAdafruitPrintCalls.size());
    if (!mockAdafruitPrintCalls.empty()) {
        TEST_ASSERT_EQUAL_STRING("Hello V2", mockAdafruitPrintCalls[0].str.c_str());
        TEST_ASSERT_EQUAL(ST77XX_WHITE, mockAdafruitPrintCalls[0].color);
    }
}

void test_print_caching(void) {
    textDisplayV2->print("Hello V2");
    int initialCalls = mockAdafruitPrintCalls.size();

    textDisplayV2->print("Hello V2"); // Should be cached
    TEST_ASSERT_EQUAL(initialCalls, mockAdafruitPrintCalls.size());

    textDisplayV2->print("World V2"); // Should update
    TEST_ASSERT_EQUAL(initialCalls + 1, mockAdafruitPrintCalls.size());
    if (!mockAdafruitPrintCalls.empty()) {
        TEST_ASSERT_EQUAL_STRING("World V2", mockAdafruitPrintCalls.back().str.c_str());
    }
}

void test_printSelectionScreen(void) {
    textDisplayV2->printSelectionScreen("Select Player", "Player 1", 0x1234);

    // Verify strings (Title, Item)
    TEST_ASSERT_EQUAL(2, mockAdafruitPrintCalls.size());
    if (mockAdafruitPrintCalls.size() >= 2) {
        TEST_ASSERT_EQUAL_STRING("Select Player", mockAdafruitPrintCalls[0].str.c_str());
        TEST_ASSERT_EQUAL_STRING("Player 1", mockAdafruitPrintCalls[1].str.c_str());
    }

    // Verify arrows (2 lines per arrow, 2 arrows = 4 lines)
    TEST_ASSERT_EQUAL(4, mockAdafruitDrawLineCalls.size());
}

void test_mode_switching(void) {
    textDisplayV2->print("Hello");
    int callsAfterPrint = mockAdafruitPrintCalls.size();

    // Switch to selection screen with same text content in item
    textDisplayV2->printSelectionScreen("Title", "Hello");
    // Should trigger update because mode changed
    TEST_ASSERT_TRUE(mockAdafruitPrintCalls.size() > callsAfterPrint);

    int callsAfterSelection = mockAdafruitPrintCalls.size();

    // Switch back to print with same text as previous print
    textDisplayV2->print("Hello");
    // Should trigger update because mode changed
    TEST_ASSERT_TRUE(mockAdafruitPrintCalls.size() > callsAfterSelection);
}

void test_selection_screen_caching(void) {
    textDisplayV2->printSelectionScreen("Title", "Item");
    int initialCalls = mockAdafruitPrintCalls.size();

    textDisplayV2->printSelectionScreen("Title", "Item");
    TEST_ASSERT_EQUAL(initialCalls, mockAdafruitPrintCalls.size());

    textDisplayV2->printSelectionScreen("Title", "Item 2");
    TEST_ASSERT_TRUE(mockAdafruitPrintCalls.size() > initialCalls);
}

void test_colorHSVtoRGB565(void) {
    // Basic test to verify compilation and some expected outputs
    // Hue 0 = Red
    uint16_t red = TextDisplayV2::colorHSVtoRGB565(0);
    // Red in RGB565 is 0xF800 (or close due to float math)
    TEST_ASSERT_TRUE((red & 0xF800) == 0xF800);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_begin);
    RUN_TEST(test_print);
    RUN_TEST(test_print_caching);
    RUN_TEST(test_printSelectionScreen);
    RUN_TEST(test_mode_switching);
    RUN_TEST(test_selection_screen_caching);
    RUN_TEST(test_colorHSVtoRGB565);
    UNITY_END();
    return 0;
}
