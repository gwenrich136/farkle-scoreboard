#ifndef MOCK_SOUND_PLAYER_H
#define MOCK_SOUND_PLAYER_H

#define SFX_VICTORY_COUNT 3

enum SoundEffect {
    SFX_SCORE_LOW = 0,
    SFX_SCORE_MID,
    SFX_SCORE_HIGH,
    SFX_BANKING,
    SFX_FARKLE,
    SFX_PENALTY_FARKLE,
    SFX_FINAL_ROUND_BELL,
    SFX_VICTORY_1,
    SFX_VICTORY_2,
    SFX_VICTORY_3,
    SFX_NONE
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
