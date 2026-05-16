#include <unity.h>
#include "MemoryCard.h"
#include "GameState.h"
#include <SD.h>
#include <ArduinoJson.h>

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

void test_MemoryCard_HasActiveGame() {
    TEST_ASSERT_FALSE(card.hasActiveGame());

    SD.mkdir("/sys");
    File f = SD.open("/sys/curr_id.txt", FILE_WRITE);
    f.println("00000042");
    f.close();

    TEST_ASSERT_TRUE(card.hasActiveGame());
}

void test_MemoryCard_LoadGameMetadata() {
    SD.mkdir("/sys");
    File fid = SD.open("/sys/curr_id.txt", FILE_WRITE);
    fid.println("00000042");
    fid.close();

    TEST_ASSERT_TRUE(card.hasActiveGame()); // This sets the internal _activeGameId

    SD.mkdir("/partial");
    SD.mkdir("/partial/00000042");
    File fmeta = SD.open("/partial/00000042/meta.jsn", FILE_WRITE);
    fmeta.print("{\"targetScore\":5000,\"players\":[{\"name\":\"Alice\",\"hue\":100},{\"name\":\"Bob\",\"hue\":200}]}");
    fmeta.close();

    GameState state;
    TEST_ASSERT_TRUE(card.loadGameMetadata(state));
    TEST_ASSERT_EQUAL_INT(5000, state.targetScore);
    TEST_ASSERT_EQUAL_INT(2, state.players.size());
    TEST_ASSERT_EQUAL_STRING("Alice", state.players[0].name.c_str());
    TEST_ASSERT_EQUAL_INT(100, state.players[0].hue);
    TEST_ASSERT_EQUAL_STRING("Bob", state.players[1].name.c_str());
    TEST_ASSERT_EQUAL_INT(200, state.players[1].hue);
}

void test_MemoryCard_ReplayGameJournal() {
    SD.mkdir("/sys");
    File fid = SD.open("/sys/curr_id.txt", FILE_WRITE);
    fid.println("00000042");
    fid.close();

    TEST_ASSERT_TRUE(card.hasActiveGame());

    GameState state;
    state.players.push_back(Player("Alice", 0));
    state.players.push_back(Player("Bob", 0));

    SD.mkdir("/partial");
    SD.mkdir("/partial/00000042");
    File fjnl = SD.open("/partial/00000042/journal.bin", FILE_WRITE);
    // Write turns:
    // Turn 1: Alice scores 300, 0 farkles
    uint32_t rec1 = TurnRecord::pack(300, 0, 0, false, false);
    fjnl.write((const uint8_t*)&rec1, sizeof(rec1));
    // Turn 2: Bob scores 500, 1 farkle
    uint32_t rec2 = TurnRecord::pack(500, 1, 1, false, false);
    fjnl.write((const uint8_t*)&rec2, sizeof(rec2));
    // Turn 3: Alice scores 400 (total 400, but the journal stores literal score bank at that time), wait, journal stores *literal banked score* as per MEMORY_FILE_TREE.md.
    // Wait, the doc says "Literal banked score. Note: Farkle rules ensure total score cannot drop below zero."
    // It means the journal stores the total absolute score of the player at the end of their turn!
    uint32_t rec3 = TurnRecord::pack(700, 0, 0, false, false); // Alice total now 700
    fjnl.write((const uint8_t*)&rec3, sizeof(rec3));
    fjnl.close();

    TEST_ASSERT_TRUE(card.replayGameJournal(state));
    TEST_ASSERT_EQUAL_INT(700, state.players[0].score);
    TEST_ASSERT_EQUAL_INT(0, state.players[0].farkle_count);
    TEST_ASSERT_EQUAL_INT(500, state.players[1].score);
    TEST_ASSERT_EQUAL_INT(1, state.players[1].farkle_count);
    // Last player to play was Alice (index 0), so next player should be Bob (index 1)
    TEST_ASSERT_EQUAL_INT(1, state.currentPlayerIndex);
}

void test_MemoryCard_ClearActiveGame() {
    SD.mkdir("/sys");
    File fid = SD.open("/sys/curr_id.txt", FILE_WRITE);
    fid.println("00000042");
    fid.close();

    TEST_ASSERT_TRUE(card.hasActiveGame());

    SD.mkdir("/partial");
    SD.mkdir("/partial/00000042");
    File fmeta = SD.open("/partial/00000042/meta.jsn", FILE_WRITE);
    fmeta.print("test");
    fmeta.close();
    File fjnl = SD.open("/partial/00000042/journal.bin", FILE_WRITE);
    fjnl.print("test");
    fjnl.close();

    card.clearActiveGame();

    TEST_ASSERT_FALSE(SD.exists("/sys/curr_id.txt"));
    TEST_ASSERT_FALSE(SD.exists("/partial/00000042/meta.jsn"));
    TEST_ASSERT_FALSE(SD.exists("/partial/00000042/journal.bin"));
    TEST_ASSERT_FALSE(SD.exists("/partial/00000042"));
    TEST_ASSERT_FALSE(card.hasActiveGame());
}

// Verifies that writeGameMetadata correctly serializes target score and players, and sets the "completed" flag to false for in-progress games.
void test_MemoryCard_WriteMetadata_SetsCompletedFalse() {
    SD.mkdir("/partial");
    SD.mkdir("/partial/00000042");

    GameState state;
    state.targetScore = 7500;
    state.players.push_back(Player("Alice", 123));

    card.writeGameMetadata(42, state);

    File f = SD.open("/partial/00000042/meta.jsn", FILE_READ);
    TEST_ASSERT_TRUE(f);

    JsonDocument doc;
    deserializeJson(doc, f);
    f.close();

    TEST_ASSERT_EQUAL_INT(7500, doc["targetScore"].as<int>());
    TEST_ASSERT_FALSE(doc["completed"].as<bool>());
    TEST_ASSERT_EQUAL_STRING("Alice", doc["players"][0]["name"].as<std::string>().c_str());
    TEST_ASSERT_EQUAL_INT(123, doc["players"][0]["hue"].as<int>());
}

// Verifies the copy-mark-delete finalization flow: copies journal.bin and rewrites meta.jsn with "completed": true into /archive/, and removes all partial files.
void test_MemoryCard_FinalizeGame_UnifiedFormat() {
    SD.mkdir("/sys");
    File fid = SD.open("/sys/curr_id.txt", FILE_WRITE);
    fid.println("00000042");
    fid.close();

    TEST_ASSERT_TRUE(card.hasActiveGame()); // sets _activeGameId = 42

    SD.mkdir("/partial");
    SD.mkdir("/partial/00000042");

    GameState state;
    state.targetScore = 10000;
    state.players.push_back(Player("Alice", 100));
    state.players.push_back(Player("Bob", 200));

    // Write partial meta
    card.writeGameMetadata(42, state);

    // Write partial journal
    File fjnl = SD.open("/partial/00000042/journal.bin", FILE_WRITE);
    uint32_t rec1 = TurnRecord::pack(500, 0, 0, false, false);
    fjnl.write((const uint8_t*)&rec1, sizeof(rec1));
    fjnl.close();

    // Finalize
    card.finalizeGame(state);

    // Assert partial cleaned up
    TEST_ASSERT_FALSE(SD.exists("/sys/curr_id.txt"));
    TEST_ASSERT_FALSE(SD.exists("/partial/00000042/meta.jsn"));
    TEST_ASSERT_FALSE(SD.exists("/partial/00000042/journal.bin"));
    TEST_ASSERT_FALSE(SD.exists("/partial/00000042"));
    TEST_ASSERT_FALSE(card.hasActiveGame());

    // Assert archive files exist
    TEST_ASSERT_TRUE(SD.exists("/archive/00000042/meta.jsn"));
    TEST_ASSERT_TRUE(SD.exists("/archive/00000042/journal.bin"));

    // Check archive metadata
    File fmeta = SD.open("/archive/00000042/meta.jsn", FILE_READ);
    JsonDocument doc;
    deserializeJson(doc, fmeta);
    fmeta.close();

    TEST_ASSERT_TRUE(doc["completed"].as<bool>());
    TEST_ASSERT_EQUAL_INT(10000, doc["targetScore"].as<int>());
    TEST_ASSERT_EQUAL_STRING("Alice", doc["players"][0]["name"].as<std::string>().c_str());

    // Check archive journal
    File farchJnl = SD.open("/archive/00000042/journal.bin", FILE_READ);
    uint32_t readRec;
    size_t bytesRead = farchJnl.read((uint8_t*)&readRec, sizeof(readRec));
    farchJnl.close();

    TEST_ASSERT_EQUAL_INT(sizeof(uint32_t), bytesRead);
    TEST_ASSERT_EQUAL_HEX32(rec1, readRec);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_MemoryCard_AutoPopulatesMissingFile);
    RUN_TEST(test_MemoryCard_SelectionNavigation);
    RUN_TEST(test_MemoryCard_ReserveAndFinalize);
    RUN_TEST(test_MemoryCard_HasActiveGame);
    RUN_TEST(test_MemoryCard_LoadGameMetadata);
    RUN_TEST(test_MemoryCard_ReplayGameJournal);
    RUN_TEST(test_MemoryCard_ClearActiveGame);
    RUN_TEST(test_MemoryCard_WriteMetadata_SetsCompletedFalse);
    RUN_TEST(test_MemoryCard_FinalizeGame_UnifiedFormat);
    return UNITY_END();
}
