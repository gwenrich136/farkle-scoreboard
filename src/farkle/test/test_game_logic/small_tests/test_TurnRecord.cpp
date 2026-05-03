#include <unity.h>
#include "MemoryCard.h"

void test_TurnRecord_pack_basic() {
    uint32_t packed = TurnRecord::pack(12345, 2, 1, false, false);
    // Score: 12345 = 0x3039 (bits 0-19)
    // Player: 2 = 0x2 (bits 20-23) -> 0x200000
    // Farkles: 1 = 0x1 (bits 24-25) -> 0x1000000
    // FinalRound: 0
    // Penalty: 0
    TEST_ASSERT_EQUAL_UINT32(0x3039 | 0x200000 | 0x1000000, packed);
}

void test_TurnRecord_pack_penalty() {
    uint32_t packed = TurnRecord::pack(1048575, 15, 2, true, true);
    // Score: 0xFFFFF
    // Player: 15 = 0xF -> 0xF00000
    // Farkles: 2 = 0x2 -> 0x2000000
    // FinalRound: 1 -> 0x4000000
    // Penalty: 1 -> 0x8000000
    TEST_ASSERT_EQUAL_UINT32(0xFFFFF | 0xF00000 | 0x2000000 | 0x4000000 | 0x8000000, packed);
}

void run_turn_record_tests() {
    RUN_TEST(test_TurnRecord_pack_basic);
    RUN_TEST(test_TurnRecord_pack_penalty);
}
