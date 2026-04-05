#ifndef MOCK_TEXT_DISPLAY_V2_H
#define MOCK_TEXT_DISPLAY_V2_H

#include <string>
#include <cstdint>

class TextDisplayV2 {
public:
    std::string captured_message;
    std::string captured_title;
    std::string captured_item;
    uint16_t captured_hue;

    TextDisplayV2(int cs, int dc, int res, int blk);
    void begin();
    void print(const char* message, uint16_t hue = 0xFFFF);
    void printSelectionScreen(const char* selectionTitle, const char* selectionItem, uint16_t hue = 0xFFFF);
};

#endif // MOCK_TEXT_DISPLAY_V2_H
