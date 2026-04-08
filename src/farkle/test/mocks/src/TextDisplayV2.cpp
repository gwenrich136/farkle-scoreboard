#include "TextDisplayV2.h"

TextDisplayV2::TextDisplayV2(int cs, int dc, int res, int blk) : captured_hue(0xFFFF) {
    // Constructor can be empty for the mock
}

void TextDisplayV2::begin() {
    // Begin can be empty for the mock
}

void TextDisplayV2::print(const char* message, uint16_t hue) {
    captured_message = message;
    captured_hue = hue;
}

void TextDisplayV2::printSelectionScreen(const char* selectionTitle, const char* selectionItem, uint16_t hue) {
    captured_title = selectionTitle;
    captured_item = selectionItem;
    captured_hue = hue;
}

void TextDisplayV2::printHeadToHeadScreen(const char* p1Place, const std::string* p1Name, uint16_t p1Hue, const char* p2Place, const std::string* p2Name, uint16_t p2Hue) {
    if (p1Name) captured_p1Name = *p1Name;
    if (p2Name) captured_p2Name = *p2Name;
    captured_p1Place = p1Place;
    captured_p1Hue = p1Hue;
    captured_p2Place = p2Place;
    captured_p2Hue = p2Hue;
}
