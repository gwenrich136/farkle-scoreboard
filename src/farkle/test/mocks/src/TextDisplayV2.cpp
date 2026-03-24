#include "TextDisplayV2.h"

TextDisplayV2::TextDisplayV2(int cs, int dc, int res, int blk) : captured_hue(0xFFFF) {
    // Constructor can be empty for the mock
}

void TextDisplayV2::begin() {
    // Begin can be empty for the mock
}

void TextDisplayV2::print(const char* message) {
    captured_message = message;
}

void TextDisplayV2::printSelectionScreen(const char* selectionTitle, const char* selectionItem, uint16_t hue) {
    captured_title = selectionTitle;
    captured_item = selectionItem;
    captured_hue = hue;
}
