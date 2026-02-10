#include "U8g2lib.h"

// Define constants
const uint8_t u8g2_font_ncenB10_tr[1] = {0};
const uint8_t u8g2_font_ncenB08_tr[1] = {0};

void u8g2_cb_r0(void) {}

// Define tracking variables
int mockU8g2BeginCount = 0;
int mockU8g2SetFontCount = 0;
std::vector<MockDrawStrCall> mockU8g2DrawStrCalls;
std::vector<MockDrawLineCall> mockU8g2DrawLineCalls;
std::string mockU8g2LastFont = "";
