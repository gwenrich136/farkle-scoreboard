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

    std::string captured_p1Place;
    std::string captured_p1Name;
    uint16_t captured_p1Hue;
    std::string captured_p2Place;
    std::string captured_p2Name;
    uint16_t captured_p2Hue;

    TextDisplayV2(int cs, int dc, int res, int blk);
    void begin();
    void print(const char* message, uint16_t hue = 0xFFFF);
    void printSelectionScreen(const char* selectionTitle, const char* selectionItem, uint16_t hue = 0xFFFF);
    void printHeadToHeadScreen(const char* p1Place, const std::string* p1Name, uint16_t p1Hue, const char* p2Place, const std::string* p2Name, uint16_t p2Hue);
};

#endif // MOCK_TEXT_DISPLAY_V2_H
