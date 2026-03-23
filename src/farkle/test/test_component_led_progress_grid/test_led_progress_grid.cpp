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
    TEST_ASSERT_EQUAL(0, (int)mockNeoPixelState.size());
}

void test_LedProgressGrid_AddPlayer(void) {
    int p1 = grid->addPlayer();
    int p2 = grid->addPlayer();

    TEST_ASSERT_EQUAL(0, p1);
    TEST_ASSERT_EQUAL(1, p2);
}

void test_LedProgressGrid_Optimization(void) {
    // This test verifies that multiple calls to update() with same state
    // do NOT trigger multiple calls to show().
    std::vector<int> scores = {1000, 2000};
    grid->addPlayer(); // p0
    grid->addPlayer(); // p1

    mockNeoPixelShowCount = 0;
    grid->update(scores.data(), (int)scores.size(), 0, 0);
    int count1 = mockNeoPixelShowCount;
    TEST_ASSERT_EQUAL(1, count1); // Should have called show() once

    grid->update(scores.data(), (int)scores.size(), 0, 0);
    int count2 = mockNeoPixelShowCount;

    // OPTIMIZED BEHAVIOR: it should NOT have incremented
    TEST_ASSERT_EQUAL(count1, count2);

    // Changing state should trigger show()
    scores[0] = 1100;
    grid->update(scores.data(), (int)scores.size(), 0, 0);
    TEST_ASSERT_EQUAL(count1 + 1, mockNeoPixelShowCount);
}

void test_LedProgressGrid_Pregame_Blink_Optimization(void) {
    grid->reset();
    mockNeoPixelShowCount = 0;

    // Advance 100ms
    advance_millis(100);
    grid->displayPlayersPregame(true);
    int count1 = mockNeoPixelShowCount;
    TEST_ASSERT_EQUAL(1, count1);

    // Call again at same time
    grid->displayPlayersPregame(true);
    TEST_ASSERT_EQUAL(count1, mockNeoPixelShowCount);

    // Advance 50ms (still < 500ms, so blink state same)
    advance_millis(50);
    grid->displayPlayersPregame(true);
    TEST_ASSERT_EQUAL(count1, mockNeoPixelShowCount);

    // Advance to 600ms (blink state changes)
    advance_millis(450); // 100 + 50 + 450 = 600
    grid->displayPlayersPregame(true);
    TEST_ASSERT_EQUAL(count1 + 1, mockNeoPixelShowCount);
}

void test_LedProgressGrid_Update_Basic(void) {
    grid->addPlayer();
    std::vector<int> scores = {5000};
    grid->update(scores.data(), (int)scores.size(), 0, 0);

    TEST_ASSERT_NOT_EQUAL(0, (int)mockNeoPixelState.size());
}

void test_LedProgressGrid_InsufficientScores(void) {
    grid->addPlayer(); // 1 player
    std::vector<int> emptyScores; // 0 scores

    mockNeoPixelShowCount = 0;
    grid->update(emptyScores.data(), (int)emptyScores.size(), 0, 0);

    TEST_ASSERT_EQUAL(0, mockNeoPixelShowCount);
}

void test_LedProgressGrid_MaxPlayers(void) {
    for (int i = 0; i < 8; ++i) {
        int idx = grid->addPlayer();
        TEST_ASSERT_EQUAL(i, idx);

        if (i < 7) {
             TEST_ASSERT_FALSE(grid->isMaxPlayersReached());
        }
    }

    TEST_ASSERT_TRUE(grid->isMaxPlayersReached());

    int idx = grid->addPlayer();
    TEST_ASSERT_EQUAL(-1, idx);
    TEST_ASSERT_TRUE(grid->isMaxPlayersReached());
}

void test_LedProgressGrid_SetTargetScore(void) {
    mockNeoPixelShowCount = 0;
    grid->addPlayer();
    std::vector<int> scores = {1000};

    grid->update(scores.data(), (int)scores.size(), 0, 0);
    int showCount1 = mockNeoPixelShowCount;
    TEST_ASSERT_EQUAL(1, showCount1);

    grid->setTargetScore(1000);
    grid->update(scores.data(), (int)scores.size(), 0, 0);

    TEST_ASSERT_EQUAL(showCount1 + 1, mockNeoPixelShowCount);
}

void test_LedProgressGrid_MaxScore_Exact(void) {
    grid->setTargetScore(10000);
    grid->addPlayer();
    std::vector<int> scores = {11000};

    grid->update(scores.data(), (int)scores.size(), 0, 0);
    TEST_ASSERT_EQUAL(11000, grid->getMaxScore());
}

void test_LedProgressGrid_MaxScore_IncludesAtRisk(void) {
    grid->setTargetScore(10000);
    grid->addPlayer();
    std::vector<int> scores = {9000};

    grid->update(scores.data(), (int)scores.size(), 0, 1500); // 9000 + 1500 = 10500
    TEST_ASSERT_EQUAL(10500, grid->getMaxScore());
}

void test_LedProgressGrid_MaxScore_Shrinking(void) {
    grid->setTargetScore(10000);
    grid->addPlayer();
    std::vector<int> scores = {12000};

    grid->update(scores.data(), (int)scores.size(), 0, 0);
    TEST_ASSERT_EQUAL(12000, grid->getMaxScore());

    scores[0] = 10000;
    grid->update(scores.data(), (int)scores.size(), 0, 2000); // 10000 + 2000 = 12000
    TEST_ASSERT_EQUAL(12000, grid->getMaxScore());

    grid->update(scores.data(), (int)scores.size(), 0, 0);
    TEST_ASSERT_EQUAL(10000, grid->getMaxScore());
}

void test_LedProgressGrid_IncludeAtRiskToggle(void) {
    grid->reset();
    grid->addPlayer();
    std::vector<int> scores = {5000};
    int atRisk = 2000;

    // 1. With includeAtRisk = true, blinkAtRisk = false
    mockNeoPixelState.clear();
    grid->update(scores.data(), (int)scores.size(), 0, atRisk, true, false);
    int stateSizeWithAtRisk = (int)mockNeoPixelState.size();
    TEST_ASSERT_NOT_EQUAL(0, stateSizeWithAtRisk);

    // 2. With includeAtRisk = false
    mockNeoPixelState.clear();
    grid->update(scores.data(), (int)scores.size(), 0, atRisk, false, false);
    int stateSizeWithoutAtRisk = (int)mockNeoPixelState.size();

    // stateSizeWithoutAtRisk should be smaller than stateSizeWithAtRisk
    // because ratio is smaller (5000/10500 vs 7000/10500)
    TEST_ASSERT(stateSizeWithoutAtRisk < stateSizeWithAtRisk);
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
    RUN_TEST(test_LedProgressGrid_IncludeAtRiskToggle);
    return UNITY_END();
}
