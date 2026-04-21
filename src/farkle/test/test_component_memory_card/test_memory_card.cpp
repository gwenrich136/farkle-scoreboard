#include <unity.h>
#include "MemoryCard.h"
#include <SD.h>

MemoryCard card(9);

void setUp() {
    SD._clearMockSD();
    card = MemoryCard(9);
}

void tearDown() {
    SD._clearMockSD();
}

void test_MemoryCard_AutoPopulatesMissingFile() {
    TEST_ASSERT_FALSE(SD.exists("players.csv"));

    TEST_ASSERT_TRUE(card.begin());

    TEST_ASSERT_TRUE(SD.exists("players.csv"));

    File f = SD.open("players.csv", FILE_READ);
    std::string line = f.readStringUntil('\n');
    if (!line.empty() && line.back() == '\r') line.pop_back();
    TEST_ASSERT_EQUAL_STRING("Geewee,0", line.c_str());
    f.close();
}

void test_MemoryCard_SelectionNavigation() {
    // create a fake file
    File f = SD.open("players.csv", FILE_WRITE);
    f.println("Alice,10");
    f.println("Bob,5");
    f.println("Charlie,15");
    f.close();

    card.beginPlayerSelection();

    // Should be sorted Charlie (15), Alice (10), Bob (5)
    TEST_ASSERT_EQUAL_STRING("Charlie", card.getCurrentPlayer()); // Initially at first player
    TEST_ASSERT_EQUAL_STRING("Alice", card.getNextPlayer());
    TEST_ASSERT_EQUAL_STRING("Bob", card.getNextPlayer());
    TEST_ASSERT_EQUAL_STRING("", card.getNextPlayer()); // End of list

    TEST_ASSERT_EQUAL_STRING("Bob", card.getPreviousPlayer()); // Go back from end to last
    TEST_ASSERT_EQUAL_STRING("Alice", card.getPreviousPlayer());
    TEST_ASSERT_EQUAL_STRING("Charlie", card.getPreviousPlayer());
    TEST_ASSERT_EQUAL_STRING("", card.getPreviousPlayer()); // Start of list
}

void test_MemoryCard_ReserveAndFinalize() {
    File f = SD.open("players.csv", FILE_WRITE);
    f.println("Alice,10");
    f.println("Bob,5");
    f.close();

    card.beginPlayerSelection();

    // Currently at Alice
    TEST_ASSERT_EQUAL_STRING("Alice", card.getCurrentPlayer());

    char nameBuf[13];
    card.reservePlayer(nameBuf); // Reserve Alice

    TEST_ASSERT_EQUAL_STRING("Alice", nameBuf);

    // Get next player should now skip Alice and go to Bob. Since reserve auto-advances, we should be at Bob.
    TEST_ASSERT_EQUAL_STRING("Bob", card.getCurrentPlayer());

    card.finalizeSelection();

    // Re-read file
    f = SD.open("players.csv", FILE_READ);
    std::string l1 = f.readStringUntil('\n');
    std::string l2 = f.readStringUntil('\n');
    f.close();

    if (!l1.empty() && l1.back() == '\r') l1.pop_back();
    if (!l2.empty() && l2.back() == '\r') l2.pop_back();

    // Alice freq should be 11, Bob 5
    TEST_ASSERT_EQUAL_STRING("Alice,11", l1.c_str());
    TEST_ASSERT_EQUAL_STRING("Bob,5", l2.c_str());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_MemoryCard_AutoPopulatesMissingFile);
    RUN_TEST(test_MemoryCard_SelectionNavigation);
    RUN_TEST(test_MemoryCard_ReserveAndFinalize);
    return UNITY_END();
}
