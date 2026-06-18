#ifndef MOCK_SOUND_PLAYER_H
#define MOCK_SOUND_PLAYER_H

#define SFX_VICTORY_COUNT 3

enum SoundEffect {
    SFX_NONE = 0,            // Sentinel — no file, no playback
    // Game sounds (enum value = file number on SD card)
    SFX_SCORE_LOW = 1,       // 0001.mp3
    SFX_SCORE_MID = 2,       // 0002.mp3
    SFX_SCORE_HIGH = 3,      // 0003.mp3
    SFX_BANKING = 4,         // 0004.mp3
    SFX_FARKLE = 5,          // 0005.mp3
    SFX_PENALTY_FARKLE = 6,  // 0006.mp3
    SFX_FINAL_ROUND_BELL = 7,// 0007.mp3
    SFX_VICTORY_1 = 8,       // 0008.mp3
    SFX_VICTORY_2 = 9,       // 0009.mp3
    SFX_VICTORY_3 = 10,      // 0010.mp3
    // System sounds
    SFX_STARTUP = 100,       // 0100.mp3
    SFX_NEW_GAME = 101,      // 0101.mp3
    SFX_RESUME_GAME = 102    // 0102.mp3
};

class SoundPlayer {
public:
    SoundPlayer() : _activeSustainingEffect(SFX_NONE) {}

    void begin() { begin_called = true; }

    void play(SoundEffect sfx) {
        last_played_effect = sfx;
        play_called = true;

        if (sfx == SFX_BANKING || sfx == SFX_FARKLE || sfx == SFX_PENALTY_FARKLE) {
            _activeSustainingEffect = sfx;
        }
        stop_called = false;
    }

    void playRandomVictory() {
        play_random_victory_called = true;
    }

    void stop() {
        if (_activeSustainingEffect != SFX_NONE) {
            stop_called = true;
            _activeSustainingEffect = SFX_NONE;
        }
    }

    // Mock tracking
    bool begin_called = false;
    bool play_called = false;
    SoundEffect last_played_effect = SFX_NONE;
    bool stop_called = false;
    bool play_random_victory_called = false;
    SoundEffect _activeSustainingEffect;
};

#endif // MOCK_SOUND_PLAYER_H
