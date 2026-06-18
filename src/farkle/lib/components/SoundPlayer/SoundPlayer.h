#ifndef SOUND_PLAYER_H
#define SOUND_PLAYER_H

/**
 * @file SoundPlayer.h
 * @brief Component driver for playing game sound effects and system chimes.
 * 
 * Responsibilities:
 * - Initialize the DFRobotDFPlayerMini MP3 hardware module via UART (Serial1).
 * - Map sound effects (represented by SoundEffect enum) directly to MP3 file numbers on the SD card.
 * - Manage playback modes (one-shot sounds, sustaining sounds with manual stop).
 * - Keep track of active sustaining effects to avoid stopping unrelated one-shot sound effects.
 */

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

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
