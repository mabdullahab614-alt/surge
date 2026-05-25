#pragma once
#include "raylib.h"

Sound mkSweep(float f0, float f1, float dur, short vol=18000);
Sound mkTone(float f, float dur, short vol=18000);
Sound mkCrunch();
Sound mkStageWin();
Music mkBGM();
Music mkBGM2();
Music mkBGM3();
Music mkBGM4();
Music mkBGM5();
Music mkBGM6();

void playSFX(Sound s, float pitchVar=0.10f);
void playLevelBGM();
bool levelBGMPlaying();
void stopAllBGM();
void pauseAllBGM();
void resumeAllBGM();
void updateActiveBGM();
