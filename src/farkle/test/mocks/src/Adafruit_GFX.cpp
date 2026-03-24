#include "Adafruit_GFX.h"

std::vector<MockAdafruitPrintCall> mockAdafruitPrintCalls;
std::vector<MockAdafruitDrawLineCall> mockAdafruitDrawLineCalls;
std::vector<MockAdafruitFillRectCall> mockAdafruitFillRectCalls;
int mockAdafruitInitCount = 0;
int mockAdafruitSetRotationCount = 0;
int mockAdafruitFillScreenCount = 0;

// Dummy Font definition
const GFXfont FreeSans9pt7b = {nullptr, nullptr, 0x20, 0x7E, 22};
