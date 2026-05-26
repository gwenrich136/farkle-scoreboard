#include "SoundPlayer.h"

SoundPlayer::SoundPlayer() : _activeSustainingEffect(SFX_NONE) {
}

void SoundPlayer::begin() {
    Serial1.begin(9600);
    // Add brief delay for DFPlayer to initialize after power-on
    delay(200);

    if (_dfPlayer.begin(Serial1)) {
        _dfPlayer.volume(20); // Hardcoded level per spec
    }
}

void SoundPlayer::play(SoundEffect sfx) {
    if (sfx == SFX_NONE) return;

    // Track sustaining effects to avoid stopping one-shots prematurely
    if (sfx == SFX_BANKING || sfx == SFX_FARKLE || sfx == SFX_PENALTY_FARKLE) {
        _activeSustainingEffect = sfx;
    }

    _dfPlayer.playFromMP3Folder((int)sfx + 1); // 1-indexed for DFPlayer
}

void SoundPlayer::playRandomVictory() {
    int index = random(SFX_VICTORY_COUNT);
    // Base victory is SFX_VICTORY_1 (7)
    play((SoundEffect)(SFX_VICTORY_1 + index));
}

void SoundPlayer::stop() {
    // Only stop if we are tracking an active sustaining effect
    if (_activeSustainingEffect != SFX_NONE) {
        _dfPlayer.stop();
        _activeSustainingEffect = SFX_NONE;
    }
}
