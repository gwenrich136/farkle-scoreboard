#include <unity.h>
#include "SoundPlayer.h"
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

SoundPlayer player;
extern MockSerial Serial1;

void setUp(void) {
    player = SoundPlayer(); // Reset
    player._dfPlayer.begin_called = false;
    player._dfPlayer.current_volume = 0;
    player._dfPlayer.last_played_file = 0;
    player._dfPlayer.stop_called = false;
}

void tearDown(void) {}

void test_begin_initializes_serial_and_volume() {
    player.begin();
    TEST_ASSERT_TRUE(player._dfPlayer.begin_called);
    TEST_ASSERT_EQUAL_UINT8(20, player._dfPlayer.current_volume);
}

void test_play_plays_file_with_offset() {
    player.play(SFX_BANKING); // SFX_BANKING = 3
    TEST_ASSERT_EQUAL_INT(4, player._dfPlayer.last_played_file);
    TEST_ASSERT_FALSE(player._dfPlayer.stop_called);
}

void test_stop_stops_active_sustaining_effect() {
    player.play(SFX_BANKING);
    TEST_ASSERT_FALSE(player._dfPlayer.stop_called);

    player.stop();
    TEST_ASSERT_TRUE(player._dfPlayer.stop_called);
}

void test_stop_ignores_one_shots() {
    player.play(SFX_SCORE_LOW);
    TEST_ASSERT_FALSE(player._dfPlayer.stop_called);

    player.stop(); // Should do nothing for one-shots
    TEST_ASSERT_FALSE(player._dfPlayer.stop_called);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_begin_initializes_serial_and_volume);
    RUN_TEST(test_play_plays_file_with_offset);
    RUN_TEST(test_stop_stops_active_sustaining_effect);
    RUN_TEST(test_stop_ignores_one_shots);
    return UNITY_END();
}
