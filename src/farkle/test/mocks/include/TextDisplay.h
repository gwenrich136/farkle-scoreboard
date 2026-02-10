#ifndef MOCK_TEXT_DISPLAY_H
#define MOCK_TEXT_DISPLAY_H

#include <string>

class TextDisplay {
public:
    std::string captured_message;
    std::string captured_title;
    std::string captured_item;

    TextDisplay();
    void begin();
    void print(const char* message);
    void printSelectionScreen(const char* selectionTitle, const char* selectionItem);
};

#endif // MOCK_TEXT_DISPLAY_H
