#include <unity.h>
#include "TextDisplay.h"
#include "U8g2lib.h"
#include "Wire.h"

// Access to mock variables from U8g2lib.cpp
extern int mockU8g2BeginCount;
extern int mockU8g2SetFontCount;
extern std::vector<MockDrawStrCall> mockU8g2DrawStrCalls;
extern std::vector<MockDrawLineCall> mockU8g2DrawLineCalls;
extern std::string mockU8g2LastFont;
extern int mockWireBeginCount;

TextDisplay* textDisplay = nullptr;

void setUp(void) {
    mockU8g2BeginCount = 0;
    mockU8g2SetFontCount = 0;
    mockU8g2DrawStrCalls.clear();
    mockU8g2DrawLineCalls.clear();
    mockWireBeginCount = 0;

    if (textDisplay) delete textDisplay;
    textDisplay = new TextDisplay();
}

void tearDown(void) {
    if (textDisplay) {
        delete textDisplay;
        textDisplay = nullptr;
    }
}

void test_begin(void) {
    textDisplay->begin();
    TEST_ASSERT_EQUAL(1, mockWireBeginCount);
    TEST_ASSERT_EQUAL(1, mockU8g2BeginCount);
}

void test_print(void) {
    textDisplay->print("Hello");
    // With nextPage mock returning 1 once, the loop runs twice (2 pages).
    TEST_ASSERT_EQUAL(2, mockU8g2DrawStrCalls.size());
    if (!mockU8g2DrawStrCalls.empty()) {
        TEST_ASSERT_EQUAL_STRING("Hello", mockU8g2DrawStrCalls[0].str.c_str());
    }
    // Verify font was set (ncenB10 is expected for print)
    TEST_ASSERT_EQUAL_STRING("ncenB10", mockU8g2LastFont.c_str());
}

void test_print_caching(void) {
    textDisplay->print("Hello");
    int initialCalls = mockU8g2DrawStrCalls.size();

    textDisplay->print("Hello"); // Should be cached
    TEST_ASSERT_EQUAL(initialCalls, mockU8g2DrawStrCalls.size());

    textDisplay->print("World"); // Should update
    // It should add 2 more calls (one for each of the 2 pages)
    TEST_ASSERT_EQUAL(initialCalls + 2, mockU8g2DrawStrCalls.size());
    if (!mockU8g2DrawStrCalls.empty()) {
        TEST_ASSERT_EQUAL_STRING("World", mockU8g2DrawStrCalls.back().str.c_str());
    }
}

void test_printSelectionScreen(void) {
    textDisplay->printSelectionScreen("Select Player", "Player 1");

    // Verify strings
    // Should draw Title then Item per page. 2 pages * 2 strings = 4 calls.
    TEST_ASSERT_EQUAL(4, mockU8g2DrawStrCalls.size());
    if (mockU8g2DrawStrCalls.size() >= 2) {
        TEST_ASSERT_EQUAL_STRING("Select Player", mockU8g2DrawStrCalls[0].str.c_str());
        TEST_ASSERT_EQUAL_STRING("Player 1", mockU8g2DrawStrCalls[1].str.c_str());
    }

    // Verify arrows
    // Should draw 2 arrows (up and down), each made of 2 lines. Total 4 lines per page.
    // 2 pages * 4 lines = 8 lines.
    TEST_ASSERT_EQUAL(8, mockU8g2DrawLineCalls.size());

    // Verify font switching
    // We expect 2 font set calls per page (Title, Main). 2 pages * 2 = 4.
    TEST_ASSERT_TRUE(mockU8g2SetFontCount >= 4);
}

void test_mode_switching(void) {
    textDisplay->print("Hello");
    int callsAfterPrint = mockU8g2DrawStrCalls.size();

    // Switch to selection screen with same text content in item
    // The cached logic should see different mode
    textDisplay->printSelectionScreen("Title", "Hello");
    // Should trigger update because mode changed
    TEST_ASSERT_TRUE(mockU8g2DrawStrCalls.size() > callsAfterPrint);

    int callsAfterSelection = mockU8g2DrawStrCalls.size();

    // Switch back to print with same text as previous print
    textDisplay->print("Hello");
    // Should trigger update because mode changed
    TEST_ASSERT_TRUE(mockU8g2DrawStrCalls.size() > callsAfterSelection);
}

void test_selection_screen_caching(void) {
    textDisplay->printSelectionScreen("Title", "Item");
    int initialCalls = mockU8g2DrawStrCalls.size();

    textDisplay->printSelectionScreen("Title", "Item");
    TEST_ASSERT_EQUAL(initialCalls, mockU8g2DrawStrCalls.size());

    textDisplay->printSelectionScreen("Title", "Item 2");
    TEST_ASSERT_TRUE(mockU8g2DrawStrCalls.size() > initialCalls);
}

void test_displayTitle(void) {
    textDisplay->displayTitle("Victory!");
    TEST_ASSERT_EQUAL(2, mockU8g2DrawStrCalls.size());
    TEST_ASSERT_EQUAL_STRING("Victory!", mockU8g2DrawStrCalls[0].str.c_str());
    TEST_ASSERT_EQUAL_STRING("ncenB18", mockU8g2LastFont.c_str());
}

void test_displayTitleWithSubtitle(void) {
    textDisplay->displayTitleWithSubtitle("Level 1", "Starting...");
    // 2 pages * 2 strings = 4 calls
    TEST_ASSERT_EQUAL(4, mockU8g2DrawStrCalls.size());
    TEST_ASSERT_EQUAL_STRING("Level 1", mockU8g2DrawStrCalls[0].str.c_str());
    TEST_ASSERT_EQUAL_STRING("Starting...", mockU8g2DrawStrCalls[1].str.c_str());
}

void test_displayTitleWithSubtitles(void) {
    textDisplay->displayTitleWithSubtitles("Game Over", "Score: 100", "Time: 5:00");
    // 2 pages * 3 strings = 6 calls
    TEST_ASSERT_EQUAL(6, mockU8g2DrawStrCalls.size());
    TEST_ASSERT_EQUAL_STRING("Game Over", mockU8g2DrawStrCalls[0].str.c_str());
    TEST_ASSERT_EQUAL_STRING("Score: 100", mockU8g2DrawStrCalls[1].str.c_str());
    TEST_ASSERT_EQUAL_STRING("Time: 5:00", mockU8g2DrawStrCalls[2].str.c_str());
}

void test_displayCharacterInput(void) {
    textDisplay->displayCharacterInput("ABC", 1); // Editing 'B'
    // Draw active char ('B'), left part ('A'), right part ('C') per page.
    // 2 pages * 3 strings = 6 calls.
    TEST_ASSERT_EQUAL(6, mockU8g2DrawStrCalls.size());

    // Check order (Active, Left, Right as per implementation)
    TEST_ASSERT_EQUAL_STRING("B", mockU8g2DrawStrCalls[0].str.c_str());
    TEST_ASSERT_EQUAL_STRING("A", mockU8g2DrawStrCalls[1].str.c_str());
    TEST_ASSERT_EQUAL_STRING("C", mockU8g2DrawStrCalls[2].str.c_str());

    // Verify arrows (2 arrows * 2 lines * 2 pages = 8 lines)
    TEST_ASSERT_EQUAL(8, mockU8g2DrawLineCalls.size());
}

void test_clamping(void) {
    // Very long string. Mock getStrWidth returns length * 10.
    // "This is a very long string that should definitely exceed 128 pixels" is > 13 chars.
    textDisplay->print("This is a very long string that will be clamped");
    // Initial x calculation: (128 - (46*10))/2 = (128 - 460)/2 = -166.
    // Clamping should set it to 0.
    TEST_ASSERT_EQUAL(0, mockU8g2DrawStrCalls[0].x);
}

void test_buffer_caching_full(void) {
    textDisplay->displayTitle("A");
    int calls = mockU8g2DrawStrCalls.size();

    textDisplay->displayTitle("A"); // Cached
    TEST_ASSERT_EQUAL(calls, mockU8g2DrawStrCalls.size());

    textDisplay->displayTitle("B"); // Redraw
    TEST_ASSERT_TRUE(mockU8g2DrawStrCalls.size() > calls);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_begin);
    RUN_TEST(test_print);
    RUN_TEST(test_print_caching);
    RUN_TEST(test_printSelectionScreen);
    RUN_TEST(test_mode_switching);
    RUN_TEST(test_selection_screen_caching);
    RUN_TEST(test_displayTitle);
    RUN_TEST(test_displayTitleWithSubtitle);
    RUN_TEST(test_displayTitleWithSubtitles);
    RUN_TEST(test_displayCharacterInput);
    RUN_TEST(test_clamping);
    RUN_TEST(test_buffer_caching_full);
    UNITY_END();
    return 0;
}
