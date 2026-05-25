#pragma once
#include "types.h"
#include <vector>

// Procedurally generate a level for the given stage (1-10) and level-within-stage (1-10).
// Fills the output vectors and sets levelWidth.
void generateLevel(int stage, int levelInStage,
                   std::vector<Platform>&   plats,
                   std::vector<Coin>&       coins,
                   std::vector<Enemy>&      enemies,
                   std::vector<Mushroom>&   mushs,
                   std::vector<Checkpoint>& cps,
                   std::vector<Spike>&      spikes,
                   std::vector<Geyser>&     geysers,
                   float&                   levelWidth);

// Maps a stage number (1-10) to a tile/visual theme index (1-6 matching old level themes).
int stageTheme(int stage);

// True for ice-physics stages (5, 9).
bool isIceStage(int stage);

// Stage name string.
const char* stageName(int stage);
