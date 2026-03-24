#include <unity.h>
#include <map>
#include <vector>
#include <iostream>
#include "LedProgressGrid.h"
#include "Adafruit_NeoPixel.h"
#include "Arduino.h"

// Access global mock state
extern std::map<uint16_t, uint32_t> mockNeoPixelState;
extern int mockNeoPixelShowCount;

LedProgressGrid* grid;

void setUp(void) {
    mockNeoPixelState.clear();
    mockNeoPixelShowCount = 0;
    // Pin 6 is common for NeoPixels
    grid = new LedProgressGrid(6);
    grid->begin();
}

void tearDown(void) {
    delete grid;
}

void test_LedProgressGrid_InitialState(void) {
    // Initial state after begin() (which calls reset())
    // Should be clear
    TEST_ASSERT_EQUAL(0, mockNeoPixelState.size());
}

void test_LedProgressGrid_AddPlayer(void) {
    int p1 = grid->addPlayer(0, 0);
    int p2 = grid->addPlayer(1, 0);

    TEST_ASSERT_EQUAL(0, p1);
    TEST_ASSERT_EQUAL(1, p2);
}

void test_LedProgressGrid_Optimization(void) {
    // This test verifies that multiple calls to update() with same state
    // do NOT trigger multiple calls to show().
    std::vector<int> scores = {1000, 2000};
    grid->addPlayer(0, 0); // p0
    grid->addPlayer(1, 0); // p1

    mockNeoPixelShowCount = 0;
    grid->update(scores.data(), scores.size(), 0, 0);
    int count1 = mockNeoPixelShowCount;
    TEST_ASSERT_EQUAL(1, count1); // Should have called show() once

    grid->update(scores.data(), scores.size(), 0, 0);
    int count2 = mockNeoPixelShowCount;

    // OPTIMIZED BEHAVIOR: it should NOT have incremented
    TEST_ASSERT_EQUAL(count1, count2);

    // Changing state should trigger show()
    scores[0] = 1100;
    grid->update(scores.data(), scores.size(), 0, 0);
    TEST_ASSERT_EQUAL(count1 + 1, mockNeoPixelShowCount);
}

void test_LedProgressGrid_Pregame_Blink_Optimization(void) {
    grid->reset();
    mockNeoPixelShowCount = 0;

    // Simulate time passing by modifying millis() mock
    // In our mock, we need to control millis.
    // displayPlayersPregame uses millis() % 1000 > 500

    // Force 100ms
    advance_millis(100);
    grid->displayPlayersPregame(0);
    int count1 = mockNeoPixelShowCount;
    TEST_ASSERT_EQUAL(1, count1);

    // Call again at same time
    grid->displayPlayersPregame(0);
    TEST_ASSERT_EQUAL(count1, mockNeoPixelShowCount);

    // Advance 50ms (still < 500ms, so blink state same)
    advance_millis(50);
    grid->displayPlayersPregame(0);
    TEST_ASSERT_EQUAL(count1, mockNeoPixelShowCount);

    // Advance to 600ms (blink state changes)
    advance_millis(450); // 100 + 50 + 450 = 600
    grid->displayPlayersPregame(0);
    TEST_ASSERT_EQUAL(count1 + 1, mockNeoPixelShowCount);
}

void test_LedProgressGrid_Update_Basic(void) {
    grid->addPlayer(0, 0);
    std::vector<int> scores = {5000};
    grid->update(scores.data(), scores.size(), 0, 0);

    // With 1 player, it uses rows 2, 3, 4, 5.
    // 5000 / 10000 = 0.5 ratio.
    // Each row is 8 pixels, so 4 pixels solid.
    // Let's check some pixels in row 2 (index 16-23 or something depending on snaking)
    // Actually get_pixel_index(row, col) is:
    // (row * 8) + (row % 2 != 0 ? col : (7 - col))
    // Row 2 is even, so (16 + (7-col))
    // col 0 -> 23
    // col 1 -> 22
    // col 2 -> 21
    // col 3 -> 20

    TEST_ASSERT_NOT_EQUAL(0, mockNeoPixelState.size());
}

void test_LedProgressGrid_InsufficientScores(void) {
    // If we have more players than scores provided, update should return early (safety check)
    grid->addPlayer(0, 0); // 1 player
    std::vector<int> emptyScores; // 0 scores

    mockNeoPixelShowCount = 0;
    grid->update(emptyScores.data(), emptyScores.size(), 0, 0);

    // Should verify that show() was NOT called because update() returned early
    TEST_ASSERT_EQUAL(0, mockNeoPixelShowCount);
}

void test_LedProgressGrid_MaxPlayers(void) {
    // Add 8 players (MAX_PLAYERS)
    for (int i = 0; i < 8; ++i) {
        int idx = grid->addPlayer(i, 0);
        TEST_ASSERT_EQUAL(i, idx);

        // Before the last one, it shouldn't be full yet?
        // Actually isMaxPlayersReached returns count >= MAX.
        // If count is 7, it returns false.
        if (i < 7) {
             TEST_ASSERT_FALSE(grid->isMaxPlayersReached());
        }
    }

    // Now it should be full (count is 8)
    TEST_ASSERT_TRUE(grid->isMaxPlayersReached());

    // Try adding 9th player
    int idx = grid->addPlayer(8, 0);
    TEST_ASSERT_EQUAL(-1, idx);
    TEST_ASSERT_TRUE(grid->isMaxPlayersReached());
}

void test_LedProgressGrid_SetTargetScore(void) {
    mockNeoPixelShowCount = 0;
    grid->addPlayer(0, 0);
    std::vector<int> scores = {1000};

    // Default target is 10000. 1000/10000 = 0.1 ratio -> 0.8 pixels (1 pixel total brightness)
    grid->update(scores.data(), scores.size(), 0, 0);
    int showCount1 = mockNeoPixelShowCount;
    TEST_ASSERT_EQUAL(1, showCount1);

    // Set target to 1000. 1000/1000 = 1.0 ratio -> 8 pixels full brightness
    grid->setTargetScore(1000);
    grid->update(scores.data(), scores.size(), 0, 0);

    // Changing target score doesn't automatically trigger show(),
    // but the next update() should see it and if it results in different pixels, it shows.
    // In this case, ratio changed from 0.1 to 1.0, so pixels definitely change.
    TEST_ASSERT_EQUAL(showCount1 + 1, mockNeoPixelShowCount);
}

void test_LedProgressGrid_MaxScore_Exact(void) {
    // Verify that _maxScore is exactly highestScore (if > _targetScore) and not a multiple of 2000.
    grid->setTargetScore(10000);
    grid->addPlayer(0, 0);
    std::vector<int> scores = {11000};

    grid->update(scores.data(), scores.size(), 0, 0);
    // Should be exactly 11000, not 12000 (which it would have been with old logic)
    TEST_ASSERT_EQUAL(11000, grid->getMaxScore());
}

void test_LedProgressGrid_MaxScore_IncludesAtRisk(void) {
    // Verify that _maxScore takes blinkingScore into account.
    grid->setTargetScore(10000);
    grid->addPlayer(0, 0);
    std::vector<int> scores = {9000};

    grid->update(scores.data(), scores.size(), 0, 1500); // 9000 + 1500 = 10500
    TEST_ASSERT_EQUAL(10500, grid->getMaxScore());
}

void test_LedProgressGrid_MaxScore_Shrinking(void) {
    // Verify that _maxScore shrinks when the leading player's score decreases.
    grid->setTargetScore(10000);
    grid->addPlayer(0, 0);
    std::vector<int> scores = {12000};

    grid->update(scores.data(), scores.size(), 0, 0);
    TEST_ASSERT_EQUAL(12000, grid->getMaxScore());

    // Player Farkles, score stays 12000? No, if blinkingScore was part of it.
    // Let's use blinkingScore to grow and then remove it.
    scores[0] = 10000;
    grid->update(scores.data(), scores.size(), 0, 2000); // 10000 + 2000 = 12000
    TEST_ASSERT_EQUAL(12000, grid->getMaxScore());

    // Now Farkle (blinkingScore becomes 0)
    grid->update(scores.data(), scores.size(), 0, 0);
    // Should shrink back to targetScore (10000) because banked is 10000 and blinkingScore is 0.
    TEST_ASSERT_EQUAL(10000, grid->getMaxScore());
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_LedProgressGrid_InitialState);
    RUN_TEST(test_LedProgressGrid_AddPlayer);
    RUN_TEST(test_LedProgressGrid_MaxPlayers);
    RUN_TEST(test_LedProgressGrid_Optimization);
    RUN_TEST(test_LedProgressGrid_Pregame_Blink_Optimization);
    RUN_TEST(test_LedProgressGrid_Update_Basic);
    RUN_TEST(test_LedProgressGrid_InsufficientScores);
    RUN_TEST(test_LedProgressGrid_SetTargetScore);
    RUN_TEST(test_LedProgressGrid_MaxScore_Exact);
    RUN_TEST(test_LedProgressGrid_MaxScore_IncludesAtRisk);
    RUN_TEST(test_LedProgressGrid_MaxScore_Shrinking);
    return UNITY_END();
}
