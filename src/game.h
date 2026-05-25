#pragma once
#include "types.h"

void spawnParticles(float x, float y, Color c, int n, float spd, float life);
void spawnPopup(float x, float y, const char* txt, Color c);
void spawnDust(float x, float y);
bool overlaps(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh);
void shake(float duration, float amount);
void resetGame();
void update(float dt);
