#ifndef DFROBOT_DFPLAYER_MINI_MOCK_H
#define DFROBOT_DFPLAYER_MINI_MOCK_H

#include <Arduino.h>

class DFRobotDFPlayerMini {
public:
    bool begin(MockSerial& serial) {
        begin_called = true;
        return true;
    }

    void volume(uint8_t volume) {
        current_volume = volume;
    }

    void playFromMP3Folder(int fileNumber) {
        last_played_file = fileNumber;
        stop_called = false;
    }

    void stop() {
        stop_called = true;
    }

    // Mock verification
    bool begin_called = false;
    uint8_t current_volume = 0;
    int last_played_file = 0;
    bool stop_called = false;
};

#endif // DFROBOT_DFPLAYER_MINI_MOCK_H
