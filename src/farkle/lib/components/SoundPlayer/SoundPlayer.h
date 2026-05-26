#ifndef SOUND_PLAYER_H
#define SOUND_PLAYER_H

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

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
    SFX_NONE // Internal use to indicate no active sound
};

class SoundPlayer {
public:
    SoundPlayer();

    void begin();
    void play(SoundEffect sfx);
    void playRandomVictory();
    void stop();

#ifdef UNIT_TEST
public:
#else
private:
#endif
    DFRobotDFPlayerMini _dfPlayer;
    SoundEffect _activeSustainingEffect;
};

#endif // SOUND_PLAYER_H
