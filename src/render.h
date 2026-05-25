#pragma once
#include "types.h"
#include "globals.h"

void initSprites();
void deinitSprites();

void fillRect(float x, float y, float w, float h, Color c);
bool drawBtn(float x, float y, float w, float h, const char* txt, Color bg, Color hov, int fs=24);
void drawCloseBtn();
void drawPlayerAt(float px, float py, int facing, int walkFrame, bool grounded, bool powered, int invFrames);
void drawDeathAnim(const Enemy& e);
void drawEnemy(const Enemy& e);
void drawHeart(float x, float y, Color c);
void drawMushroom(const Mushroom& m);
void drawCheckpoint(const Checkpoint& cp);
void drawPlatform(const Platform& p);
void drawFlag();
void drawMenu();
void drawPause();
void drawLevelComplete();
void drawStageMap();
void drawStageComplete();
void drawGameWinner();
void drawScene();
void drawEditor();
void drawSettings();
