#include <unity.h>
#include <map>
#include <vector>
#include "FarkleWarningLights.h"
#include "PlayerLayout.h"
#include "Adafruit_NeoPixel.h"
#include "Arduino.h"

// Access global mock state
extern std::map<uint16_t, uint32_t> mockNeoPixelState;
extern int mockNeoPixelShowCount;

FarkleWarningLights* lights;

void setUp(void) {
    mockNeoPixelState.clear();
    mockNeoPixelShowCount = 0;
    lights = new FarkleWarningLights(A1);
    lights->begin();
}

void tearDown(void) {
    delete lights;
}

void test_Update_SetsCorrectColorsAndBrightness(void) {
    // 4 Players
    int farkleCounts[] = {0, 1, 2, 0}; // P0: 0 (Active), P1: 1, P2: 2, P3: 0
    int playerCount = 4;
    int blinkingPlayerIndex = 0; // P0 is blinking
    bool isBlinking = true;

    // Player 0 (Blinking, 0 Farkles) -> White (50% brightness = 128)
    // Player 1 (Idle, 1 Farkle) -> Yellow (50% brightness = 128)
    // Player 2 (Idle, 2 Farkles) -> Red (50% brightness = 128)
    // Player 3 (Idle, 0 Farkles) -> Off

    lights->update(farkleCounts, playerCount, blinkingPlayerIndex, isBlinking);

    // P0: Indices 0, 1. Active. 0 Farkles -> White. Brightness 128.
    // Expect: h=0, s=0, v=128. -> 0x00000080
    TEST_ASSERT_EQUAL_HEX32(0x00000080, mockNeoPixelState[0]);
    TEST_ASSERT_EQUAL_HEX32(0x00000080, mockNeoPixelState[1]);

    // P1: Indices 2, 3. Idle. 1 Farkle -> Yellow. Brightness 128.
    // Yellow Hue is 10922.
    // Expect: h=10922, s=255, v=128. -> 0x2AAAFF80
    TEST_ASSERT_EQUAL_HEX32(0x2AAAFF80, mockNeoPixelState[2]);
    TEST_ASSERT_EQUAL_HEX32(0x2AAAFF80, mockNeoPixelState[3]);

    // P2: Indices 4, 5. Idle. 2 Farkles -> Red. Brightness 128.
    // Red Hue is 0.
    // Expect: h=0, s=255, v=128. -> 0x0000FF80
    TEST_ASSERT_EQUAL_HEX32(0x0000FF80, mockNeoPixelState[4]);
    TEST_ASSERT_EQUAL_HEX32(0x0000FF80, mockNeoPixelState[5]);

    // P3: Indices 6, 7. Idle. 0 Farkles -> Off.
    // Off -> 0
    TEST_ASSERT_EQUAL_HEX32(0x00000000, mockNeoPixelState[6]);
    TEST_ASSERT_EQUAL_HEX32(0x00000000, mockNeoPixelState[7]);
}

void test_Update_NoBlinkingPlayer_AllSolid(void) {
    // 4 Players, No one blinking (e.g. Banking phase)
    int farkleCounts[] = {0, 1, 2, 0};
    int playerCount = 4;
    int blinkingPlayerIndex = -1; // No one blinking
    bool isBlinking = true; // Should depend on valid index

    lights->update(farkleCounts, playerCount, blinkingPlayerIndex, isBlinking);

    // P0: Idle. 0 Farkles -> Off. (Was White when active)
    TEST_ASSERT_EQUAL_HEX32(0x00000000, mockNeoPixelState[0]);
    TEST_ASSERT_EQUAL_HEX32(0x00000000, mockNeoPixelState[1]);

    // P1: Idle. 1 Farkle -> Yellow (128).
    TEST_ASSERT_EQUAL_HEX32(0x2AAAFF80, mockNeoPixelState[2]);

    // P2: Idle. 2 Farkles -> Red (128).
    TEST_ASSERT_EQUAL_HEX32(0x0000FF80, mockNeoPixelState[4]);

    // P3: Idle. 0 Farkles -> Off.
    TEST_ASSERT_EQUAL_HEX32(0x00000000, mockNeoPixelState[6]);
}

void test_BlinkLogic(void) {
    int farkleCounts[] = {0, 0};
    int playerCount = 2;
    int blinkingPlayerIndex = 0;
    bool isBlinking = false; // OFF phase

    lights->update(farkleCounts, playerCount, blinkingPlayerIndex, isBlinking);

    // P0: Active, 0 Farkles. Blink OFF phase -> Color 0 (OFF).
    TEST_ASSERT_EQUAL_HEX32(0, mockNeoPixelState[0]);

    // Enable Blink
    isBlinking = true;
    lights->update(farkleCounts, playerCount, blinkingPlayerIndex, isBlinking);

    // P0: Active, 0 Farkles. Blink ON phase -> White 128.
    TEST_ASSERT_EQUAL_HEX32(0x00000080, mockNeoPixelState[0]);
}

void test_Alternate_SmoothTransition(void) {
    // 4 Players, P0 is current
    int playerCount = 4;
    int currentPlayerIndex = 0;

    // Align to start of a second
    unsigned long current = millis();
    unsigned long offset = 1000 - (current % 1000);
    advance_millis(offset);
    // Now millis() % 1000 == 0

    // T=0ms: Red Phase (0-400ms)
    lights->alternate(currentPlayerIndex, playerCount);
    // Expect Red (Hue 0, Sat 255, Val 255) -> 0x0000FFFF
    TEST_ASSERT_EQUAL_HEX32(0x0000FFFF, mockNeoPixelState[0]);
    TEST_ASSERT_EQUAL_HEX32(0x0000FFFF, mockNeoPixelState[1]);

    // T=399ms: Still Red
    advance_millis(399);
    lights->alternate(currentPlayerIndex, playerCount);
    TEST_ASSERT_EQUAL_HEX32(0x0000FFFF, mockNeoPixelState[0]);

    // T=450ms: Mid-Transition Red->Yellow (400-500ms)
    // map(450, 400, 500, 0, 10922) -> 5461
    // Color: h=5461, s=255, v=255 -> 0x1555FFFF
    advance_millis(51); // 399 + 51 = 450
    lights->alternate(currentPlayerIndex, playerCount);
    TEST_ASSERT_EQUAL_HEX32(0x1555FFFF, mockNeoPixelState[0]);

    // T=500ms: Yellow Phase (500-900ms)
    // Hue 10922 -> 0x2AAAFFFF
    advance_millis(50); // 450 + 50 = 500
    lights->alternate(currentPlayerIndex, playerCount);
    TEST_ASSERT_EQUAL_HEX32(0x2AAAFFFF, mockNeoPixelState[0]);

    // T=899ms: Still Yellow
    advance_millis(399); // 500 + 399 = 899
    lights->alternate(currentPlayerIndex, playerCount);
    TEST_ASSERT_EQUAL_HEX32(0x2AAAFFFF, mockNeoPixelState[0]);

    // T=950ms: Mid-Transition Yellow->Red (900-1000ms)
    // map(950, 900, 1000, 10922, 0) -> 5461
    // Color: 0x1555FFFF
    advance_millis(51); // 899 + 51 = 950
    lights->alternate(currentPlayerIndex, playerCount);
    TEST_ASSERT_EQUAL_HEX32(0x1555FFFF, mockNeoPixelState[0]);

    // T=1000ms: Back to Red
    advance_millis(50); // 950 + 50 = 1000 (0 mod 1000)
    lights->alternate(currentPlayerIndex, playerCount);
    TEST_ASSERT_EQUAL_HEX32(0x0000FFFF, mockNeoPixelState[0]);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_Update_SetsCorrectColorsAndBrightness);
    RUN_TEST(test_Update_NoBlinkingPlayer_AllSolid);
    RUN_TEST(test_BlinkLogic);
    RUN_TEST(test_Alternate_SmoothTransition);
    return UNITY_END();
}
