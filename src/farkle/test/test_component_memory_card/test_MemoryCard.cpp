#include <unity.h>
#include "MemoryCard.h"
#include "SD.h"

int unity_fail_count = 0;

MemoryCard memoryCard(9);

void setUp(void) {
    SD = SDClass(); // Reset the mock SD state
}

void tearDown(void) {
}

void test_MemoryCard_InitializationCreatesDefaultFile(void) {
    TEST_ASSERT_FALSE(SD.exists("players.csv"));

    bool result = memoryCard.begin();
    TEST_ASSERT_TRUE(result);

    TEST_ASSERT_TRUE(SD.exists("players.csv"));
    std::string content = SD.mockGetFileContent("players.csv");
    TEST_ASSERT_TRUE(content.find("Geewee,0") != std::string::npos);
    TEST_ASSERT_TRUE(content.find("Andrea,0") != std::string::npos);
}

void test_MemoryCard_NavigationAndSelection(void) {
    SD.mockSetFileContent("players.csv", "Alpha,5\nBravo,2\nCharlie,0\n");

    memoryCard.begin();
    memoryCard.beginPlayerSelection();

    const char* p1 = memoryCard.getNextPlayer();
    TEST_ASSERT_EQUAL_STRING("Alpha", p1);

    const char* p2 = memoryCard.getNextPlayer();
    TEST_ASSERT_EQUAL_STRING("Bravo", p2);

    char reserved[13];
    memoryCard.reservePlayer(reserved);
    TEST_ASSERT_EQUAL_STRING("Bravo", reserved);

    const char* p3 = memoryCard.getNextPlayer();
    TEST_ASSERT_EQUAL_STRING("Charlie", p3);

    const char* pOut = memoryCard.getNextPlayer();
    TEST_ASSERT_EQUAL_STRING("", pOut);
}

void test_MemoryCard_FinalizeSelectionWritesFrequencies(void) {
    SD.mockSetFileContent("players.csv", "Alpha,5\nBravo,2\nCharlie,0\n");

    memoryCard.begin();
    memoryCard.beginPlayerSelection();

    memoryCard.getNextPlayer(); // Alpha
    char res[13];
    memoryCard.reservePlayer(res); // Reserve Alpha

    memoryCard.getNextPlayer(); // Bravo
    memoryCard.getNextPlayer(); // Charlie
    memoryCard.reservePlayer(res); // Reserve Charlie

    memoryCard.finalizeSelection();

    std::string content = SD.mockGetFileContent("players.csv");

    // Alpha should now be 6, Charlie should be 1
    TEST_ASSERT_TRUE(content.find("Alpha,6") != std::string::npos);
    TEST_ASSERT_TRUE(content.find("Bravo,2") != std::string::npos);
    TEST_ASSERT_TRUE(content.find("Charlie,1") != std::string::npos);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_MemoryCard_InitializationCreatesDefaultFile);
    RUN_TEST(test_MemoryCard_NavigationAndSelection);
    RUN_TEST(test_MemoryCard_FinalizeSelectionWritesFrequencies);
    return UNITY_END();
}
