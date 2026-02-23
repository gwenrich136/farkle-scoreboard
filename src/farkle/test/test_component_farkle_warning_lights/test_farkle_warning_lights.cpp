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
    int currentPlayerIndex = 0;
    bool isBlinking = true;

    // Player 0 (Active, 0 Farkles) -> White (255, 255, 255)
    // Player 1 (Idle, 1 Farkle) -> Yellow (255, 255, 0) but Dim (50 brightness)
    // Player 2 (Idle, 2 Farkles) -> Red (255, 0, 0) but Dim
    // Player 3 (Idle, 0 Farkles) -> Off? Wait, 0 Farkles: White (Active only; others are OFF).

    lights->update(farkleCounts, playerCount, currentPlayerIndex, isBlinking);

    // Verify P0 (Active, 0 Farkles) -> White Full Brightness
    // P0 maps to LEDs 0 and 1 (4 players -> 2 LEDs each)
    // White: RGB(255, 255, 255) or HSV?
    // Implementation uses HSV likely? "White (Active player only)".
    // If using HSV, White is Saturation 0.

    // Let's assume implementation uses setPixelColor with RGB or HSV converted.
    // The mock NeoPixel stores uint32_t.
    // If implementation uses ColorHSV(hue, sat, val), the mock stores it as ((uint32_t)h << 16) | ((uint32_t)s << 8) | v;

    // We need to know how the component implements colors.
    // Plan says: "White (Active player only; others are OFF)."
    // "1 Farkle: Yellow."
    // "2+ Farkles: Red."

    // Colors:
    // White: Hue?, Sat 0, Val 255 (Full)
    // Yellow: Hue ~10922 (65536/6), Sat 255, Val ~50 (Dim)
    // Red: Hue 0, Sat 255, Val ~50 (Dim)

    // Since I haven't implemented the component yet, I define the expectation here.
    // I should implement component to match test.

    // Let's assume ColorHSV usage.
    // White: ColorHSV(0, 0, 255) -> 0x000000FF
    // Yellow: ColorHSV(10922, 255, 50) -> 0x2AABFF32 (approx)
    // Red: ColorHSV(0, 255, 50) -> 0x0000FF32

    // Actually, I can check specific properties if I know the encoding.
    // Mock ColorHSV: ((uint32_t)h << 16) | ((uint32_t)s << 8) | v

    // P0: Indices 0, 1. Active. 0 Farkles -> White. Brightness 255.
    // Expect: h=0, s=0, v=255. -> 0x000000FF
    TEST_ASSERT_EQUAL_HEX32(0x000000FF, mockNeoPixelState[0]);
    TEST_ASSERT_EQUAL_HEX32(0x000000FF, mockNeoPixelState[1]);

    // P1: Indices 2, 3. Idle. 1 Farkle -> Yellow. Brightness 50.
    // Yellow Hue is typically 65536/6 = 10922.
    // Expect: h=10922, s=255, v=50. -> 0x2AABFF32
    TEST_ASSERT_EQUAL_HEX32(0x2AAAFF32, mockNeoPixelState[2]); // 10922 is 0x2AAA
    TEST_ASSERT_EQUAL_HEX32(0x2AAAFF32, mockNeoPixelState[3]);

    // P2: Indices 4, 5. Idle. 2 Farkles -> Red. Brightness 50.
    // Red Hue is 0.
    // Expect: h=0, s=255, v=50. -> 0x0000FF32
    TEST_ASSERT_EQUAL_HEX32(0x0000FF32, mockNeoPixelState[4]);
    TEST_ASSERT_EQUAL_HEX32(0x0000FF32, mockNeoPixelState[5]);

    // P3: Indices 6, 7. Idle. 0 Farkles -> Off.
    // Off -> 0
    TEST_ASSERT_EQUAL_HEX32(0x00000000, mockNeoPixelState[6]); // Or check if it exists? Map creates on access?
    // Mock setPixelColor creates entry. If cleared/off, maybe it sets to 0.
    // Let's assume implementation sets it to 0.
    TEST_ASSERT_EQUAL_HEX32(0x00000000, mockNeoPixelState[7]);
}

void test_MultiLedMapping(void) {
    // 2 Players. P0 -> Indices 0, 1, 2. P1 -> Indices 5, 6, 7. (Based on PlayerLayout logic)
    // PlayerLayout: 2 players -> P0: start 0, len 3. P1: start 5, len 3.
    // Indices 3, 4 should be unused/off.

    int farkleCounts[] = {1, 2};
    int playerCount = 2;
    int currentPlayerIndex = 0; // P0 Active
    bool isBlinking = true;

    lights->update(farkleCounts, playerCount, currentPlayerIndex, isBlinking);

    // P0: Active, 1 Farkle -> Yellow, Full Brightness (255)
    // Yellow: h=10922, s=255, v=255 -> 0x2AAAFFFF
    TEST_ASSERT_EQUAL_HEX32(0x2AAAFFFF, mockNeoPixelState[0]);
    TEST_ASSERT_EQUAL_HEX32(0x2AAAFFFF, mockNeoPixelState[1]);
    TEST_ASSERT_EQUAL_HEX32(0x2AAAFFFF, mockNeoPixelState[2]);

    // P1: Idle, 2 Farkles -> Red, Dim Brightness (50)
    // Red: h=0, s=255, v=50 -> 0x0000FF32
    TEST_ASSERT_EQUAL_HEX32(0x0000FF32, mockNeoPixelState[5]);
    TEST_ASSERT_EQUAL_HEX32(0x0000FF32, mockNeoPixelState[6]);
    TEST_ASSERT_EQUAL_HEX32(0x0000FF32, mockNeoPixelState[7]);

    // Unused pixels 3, 4 should be 0
    // Note: Implementation should probably clear or set them to 0.
    // If I use clear() at start of update, they are removed from map?
    // Mock clear() clears the map.
    // If setPixelColor is not called for 3, 4, they won't be in map.
    // So checking map size or absence.
    // Or if implementation sets them to 0 explicitly.
    // Let's check that they are not set to any color (or 0).
    if (mockNeoPixelState.count(3)) {
        TEST_ASSERT_EQUAL_HEX32(0, mockNeoPixelState[3]);
    }
    if (mockNeoPixelState.count(4)) {
        TEST_ASSERT_EQUAL_HEX32(0, mockNeoPixelState[4]);
    }
}

void test_BlinkLogic(void) {
    // Verify that when isBlinking is false, Active Player is OFF (or Dim?)
    // Requirement: "Current Player: Full Brightness (255), Blinking (sync with 500ms global timer)."
    // "Blinking" usually means On/Off.
    // So if isBlinking is false (OFF phase), Active Player should be OFF?
    // Or does "Blinking" mean toggling between Full and Dim?
    // Usually "Blink" means On/Off.
    // Let's assume On/Off for Active Player.

    int farkleCounts[] = {0, 0};
    int playerCount = 2;
    int currentPlayerIndex = 0;
    bool isBlinking = false; // OFF phase

    lights->update(farkleCounts, playerCount, currentPlayerIndex, isBlinking);

    // P0: Active, 0 Farkles. Blink OFF phase -> Color 0 (OFF).
    TEST_ASSERT_EQUAL_HEX32(0, mockNeoPixelState[0]);

    // Enable Blink
    isBlinking = true;
    lights->update(farkleCounts, playerCount, currentPlayerIndex, isBlinking);

    // P0: Active, 0 Farkles. Blink ON phase -> White Full.
    TEST_ASSERT_EQUAL_HEX32(0x000000FF, mockNeoPixelState[0]);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_Update_SetsCorrectColorsAndBrightness);
    RUN_TEST(test_MultiLedMapping);
    RUN_TEST(test_BlinkLogic);
    return UNITY_END();
}
