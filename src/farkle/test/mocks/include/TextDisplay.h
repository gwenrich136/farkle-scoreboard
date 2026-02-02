#ifndef MOCK_TEXT_DISPLAY_H
#define MOCK_TEXT_DISPLAY_H

#include <string>

class TextDisplay {
public:
    std::string captured_message;

    TextDisplay();
    void begin();
    void print(const char* message);
};

#endif // MOCK_TEXT_DISPLAY_H
