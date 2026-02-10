#include "TextDisplay.h"

TextDisplay::TextDisplay() {
    // Constructor can be empty for the mock
}

void TextDisplay::begin() {
    // Begin can be empty for the mock
}

void TextDisplay::print(const char* message) {
    captured_message = message;
}

void TextDisplay::printSelectionScreen(const char* selectionTitle, const char* selectionItem) {
    captured_title = selectionTitle;
    captured_item = selectionItem;
}
