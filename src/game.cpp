#include "game.h"
#include "globals.h"
#include "audio.h"
#include "levels.h"
#include "procgen.h"
#include "Tuning.h"
#include <cmath>
#include <algorithm>
#include <fstream>

// Compute and save grade, set scorecard display vars, transition to LevelComplete
static void finishLevel() {
    int coinsGot = 0, coinsTotal = (int)g.coins.size();
    for (const auto& c : g.coins) if (c.collected) coinsGot++;
    float coinPct = coinsTotal > 0 ? (float)coinsGot / coinsTotal : 1.f;
    int grade;
    if (g.levelDeaths == 0 && coinPct >= 0.75f && g.maxCombo >= 3) grade = 4; // S
    else if (g.levelDeaths <= 1 && coinPct >= 0.5f)                grade = 3; // A
    else if (g.levelDeaths <= 4)                                    grade = 2; // B
    else                                                            grade = 1; // C
    g.levelGrades[g.currentStage][g.levelInStage] =
        std::max(g.levelGrades[g.currentStage][g.levelInStage], grade);
    g.stageProgress[g.currentStage] =
        std::max(g.stageProgress[g.currentStage], g.levelInStage);
    g.lastGrade      = grade;
    g.lastDeaths     = g.levelDeaths;
    g.lastLevelTime  = g.elapsed - g.levelStartTime;
    g.lastCoinPct    = coinPct;
    g.lastCoinsGot   = coinsGot;
    g.lastCoinsTotal = coinsTotal;
    g.lastMaxCombo   = g.maxCombo;
    if (g.score > g.highScore) g.highScore = g.score;
    saveRecords();
    g.levelCompleteTimer = 0.f;
    g.screen = Screen::LevelComplete;
}

static void unlockAch(int bit, const char* name) {
    if (g.achievementFlags & (1 << bit)) return;
    g.achievementFlags |= (1 << bit);
    AchNotif n{}; snprintf(n.name, sizeof(n.name), "%s", name); n.life = 3.5f;
    g.achievementNotifs.push_back(n);
    saveRecords();
}

void spawnParticles(float x, float y, Color c, int n, float spd, float life) {
    for (int i = 0; i < n; i++) {
        float a = GetRandomValue(0, 360) * PI / 180.f, s = GetRandomValue(60, 120) / 100.f * spd;
        g.particles.push_back({x, y, s*cosf(a), s*sinf(a), life, life, (float)GetRandomValue(2,5), {c.r,c.g,c.b,255}});
    }
}

void spawnDust(float x, float y) {
    for (int i = 0; i < 6; i++) {
        float a = (140.f + GetRandomValue(0, 80)) * PI / 180.f;
        g.particles.push_back({x+14, y+36, cosf(a)*2.f, sinf(a)*1.5f, .3f, .3f, 3, {180,180,160,200}});
    }
}

bool overlaps(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
    return ax < bx+bw && ax+aw > bx && ay < by+bh && ay+ah > by;
}

void shake(float duration, float amount) { g.shakeTimer = duration; g.shakeAmount = amount; g.shakeDecay = duration; }

void spawnPopup(float x, float y, const char* txt, Color c) {
    Popup p{}; p.x = x; p.y = y; p.vy = -1.8f; p.life = 1.1f; p.maxLife = 1.1f; p.col = c;
    snprintf(p.text, sizeof(p.text), "%s", txt);
    g.popups.push_back(p);
}

static void chainKill(float ex, float ey, Color /*baseCol*/) {
    g.combo++;
    g.comboTimer = CHAIN_WINDOW;
    if (g.combo > g.maxCombo) g.maxCombo = g.combo;

    Color pCol; int extraStop; float shakeAmt, shakeDur;
    if (g.combo >= 10) {
        pCol = {255, 255, 255, 255};
        extraStop = 2; shakeAmt = 14.f; shakeDur = 0.6f;
        g.timeScale      = CHAIN_TIMESCALE;
        g.timeScaleTimer = CHAIN_TIMESCALE_DUR;
        g.flashTimer     = 0.12f;
        g.bgmDipTimer    = 0.4f;
        if (g.audioReady) for (int i = 1; i <= 6; i++) if (IsMusicStreamPlaying(g.bgm[i])) SetMusicVolume(g.bgm[i], g.musicVolume * 0.25f);
        spawnPopup(ex, ey - 24, "INSANE!", {255, 50, 200, 255});
    } else if (g.combo >= 6) {
        pCol = {255, 0, 180, 255};
        extraStop = 2; shakeAmt = 8.f; shakeDur = 0.4f;
        g.vignettePulse = 0.8f;
    } else if (g.combo >= 3) {
        pCol = {0, 200, 255, 255};
        extraStop = 1; shakeAmt = 5.f; shakeDur = 0.25f;
    } else {
        pCol = {255, 255, 255, 255};
        extraStop = 0; shakeAmt = 3.f; shakeDur = 0.15f;
    }

    spawnParticles(ex, ey, pCol, 8, 3.f, .4f);
    g.hitStop = 4 + extraStop;
    shake(shakeDur, shakeAmt);

    // Pitch rises per combo depth — "ding ding ding" ladder
    if (g.audioReady) {
        float p = std::min(1.f + (g.combo - 1) * 0.09f, 2.6f);
        SetSoundPitch(g.sfxCoin, p);
        PlaySound(g.sfxCoin);
    }

    int pts = 50 * std::min(g.combo, 8);
    g.score += pts;
    char popTxt[24];
    if (g.combo > 1) snprintf(popTxt, sizeof(popTxt), "x%d  +%d", g.combo, pts);
    else             snprintf(popTxt, sizeof(popTxt), "+%d", pts);
    Color popCol = g.combo >= 10 ? Color{255, 50, 200, 255}
                 : g.combo >= 6  ? Color{255,  0, 180, 255}
                 : g.combo >= 3  ? Color{  0, 200, 255, 255}
                 :                 Color{255, 255, 200, 255};
    spawnPopup(ex, ey, popTxt, popCol);
}

void resetGame() {
    g.score = 0; g.lives = 3; g.elapsed = 0;
    g.currentStage = 1; g.levelInStage = 1; g.currentLevel = 1;
    g.combo = 0; g.comboTimer = 0.f; g.maxCombo = 0;
    g.timeScale = 1.f; g.timeScaleTimer = 0.f; g.vignettePulse = 0.f;
    g.cameraLookahead = 0.f;
    g.stageMapTimer = 0.f;
    g.screen = Screen::StageMap;
    initLevel();
    playLevelBGM();
}

void update(float dt) {
    if (g.hitStop > 0) { g.hitStop--; return; }

    if (g.bgmDipTimer > 0.f) {
        g.bgmDipTimer -= dt;
        if (g.bgmDipTimer <= 0.f) { g.bgmDipTimer = 0.f; applyVolumes(); }
    }

    // Normalize all position deltas to 60-fps equivalent so physics is
    // frame-rate independent while keeping the same feel at the target 60 fps.
    const float DT60 = dt * 60.f;

    if (!levelBGMPlaying()) playLevelBGM();
    if (g.flashTimer    > 0.f) g.flashTimer  -= dt;
    if (g.vignettePulse > 0.f) g.vignettePulse = std::max(0.f, g.vignettePulse - dt * 2.f);
    if (g.comboTimer    > 0.f) { g.comboTimer -= dt; if (g.comboTimer <= 0.f) g.combo = 0; }
    if (g.player.squashTimer > 0.f) g.player.squashTimer = std::max(0.f, g.player.squashTimer - dt);
    g.elapsed += dt;
    if (g.shakeTimer > 0) g.shakeTimer = std::max(0.f, g.shakeTimer - dt);
    if (g.player.wallCooldown > 0.f) g.player.wallCooldown -= dt;

    bool gpConn = IsGamepadAvailable(0);
    bool left  = IsKeyDown(g.keyLeft)  || IsKeyDown(KEY_A)
              || (gpConn && GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) < -.25f);
    bool right = IsKeyDown(g.keyRight) || IsKeyDown(KEY_D)
              || (gpConn && GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) >  .25f);
    bool jdn   = IsKeyDown(g.keyJump) || IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)
              || (gpConn && IsGamepadButtonDown(0,  GAMEPAD_BUTTON_RIGHT_FACE_DOWN));
    bool jpr   = IsKeyPressed(g.keyJump) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)
              || (gpConn && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));
    bool dUp   = IsKeyDown(KEY_UP)   || IsKeyDown(KEY_W)
              || (gpConn && GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) < -.25f);
    bool dDown = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)
              || (gpConn && GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) >  .25f);
    if (jpr) g.player.jumpBuffer = 8; else if (g.player.jumpBuffer > 0) g.player.jumpBuffer--;

    float tvx = right ? MOVE_SPEED : left ? -MOVE_SPEED : 0.f;
    if (gpConn) {
        float axisX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        float curved = fabsf(axisX) > 0.1f ? axisX * fabsf(axisX) : 0.f;
        if (fabsf(curved) > 0.01f) tvx = curved * MOVE_SPEED;
    }
    if (isIceStage(g.currentStage) && g.player.grounded)
        // Icy surface: exponential approach so sliding scales correctly with dt
        g.player.vx += (tvx - g.player.vx) * (1.f - powf(0.9f, DT60));
    else
        g.player.vx = tvx;
    if (right) g.player.facing =  1;
    if (left)  g.player.facing = -1;

    // Wind (stages 5, 6, 9) — persistent horizontal push while airborne
    if (g.windForce != 0.f && !g.player.grounded) {
        g.player.vx += g.windForce * DT60;
        g.player.vx = std::max(-MOVE_SPEED * 1.8f, std::min(g.player.vx, MOVE_SPEED * 1.8f));
    }

    // Dash
    bool dsh = IsKeyPressed(g.keyDash) || IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)
            || (gpConn && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT));
    if (g.player.dashCooldown > 0.f) g.player.dashCooldown -= dt;
    if (dsh && g.player.dashCooldown <= 0.f && g.player.dashTimer <= 0.f) {
        float dx = right ? 1.f : left ? -1.f : (float)g.player.facing;
        float dy = dUp ? -1.f : dDown ? 1.f : 0.f;
        if (dx != 0.f && dy != 0.f) { dx *= 0.7071f; dy *= 0.7071f; }
        g.player.dashDirX    = dx;
        g.player.dashDirY    = dy;
        g.player.dashTimer   = DASH_DURATION;
        g.player.dashCooldown = DASH_COOLDOWN;
        g.player.invFrames   = DASH_IFRAMES;
        g.player.aiCount     = 0;
        if (g.audioReady) playSFX(g.sfxDash, .05f);
        spawnParticles(g.player.x + g.player.w/2, g.player.y + g.player.h/2, {100,200,255,255}, 10, 3.5f, .28f);
    }
    if (g.player.dashTimer > 0.f) {
        float spd = MOVE_SPEED * DASH_SPEED_MULT;
        g.player.vx = g.player.dashDirX * spd;
        g.player.vy = g.player.dashDirY * spd;
        g.player.dashTimer = std::max(0.f, g.player.dashTimer - dt);
        for (int i = 4; i > 0; i--) { g.player.aiX[i] = g.player.aiX[i-1]; g.player.aiY[i] = g.player.aiY[i-1]; }
        g.player.aiX[0] = g.player.x; g.player.aiY[0] = g.player.y;
        g.player.aiCount = std::min(g.player.aiCount + 1, AFTERIMAGE_COUNT);
    } else {
        g.player.aiCount = 0;
    }

    if (g.player.vx != 0 && g.player.grounded) {
        if (++g.player.walkTimer > 8) { g.player.walkFrame = (g.player.walkFrame + 1) % 4; g.player.walkTimer = 0; }
    } else { g.player.walkFrame = 0; g.player.walkTimer = 0; }

    // Wall contact
    bool wallL = false, wallR = false;
    if (!g.player.grounded && g.player.coyoteFrames <= 0 && g.player.wallCooldown <= 0.f) {
        for (const auto& p : g.platforms) {
            if (left  && overlaps(g.player.x-4, g.player.y+4, 5, g.player.h-8, p.x, p.y, p.w, p.h)) wallL = true;
            if (right && overlaps(g.player.x+g.player.w-1, g.player.y+4, 5, g.player.h-8, p.x, p.y, p.w, p.h)) wallR = true;
        }
    }
    bool onWall = wallL || wallR;
    if (onWall && g.player.vy > 2.f) g.player.vy = std::min(g.player.vy, 3.2f);

    bool prevGnd = g.player.grounded, canJ = (g.player.grounded || g.player.coyoteFrames > 0) && !g.player.jumpHeld;
    if (g.player.jumpBuffer > 0 && canJ) {
        g.player.vy = JUMP_FORCE; g.player.grounded = false; g.player.doubleJumped = false;
        g.player.jumpBuffer = 0; g.player.coyoteFrames = 0;
        if (g.audioReady) playSFX(g.sfxJump);
    } else if (jpr && onWall && !g.player.jumpHeld) {
        g.player.vy = JUMP_FORCE * .85f;
        g.player.vx = wallL ? MOVE_SPEED * 1.8f : -MOVE_SPEED * 1.8f;
        g.player.facing = wallL ? 1 : -1;
        g.player.doubleJumped = false; g.player.wallCooldown = 0.28f; g.player.jumpBuffer = 0;
        if (g.audioReady) playSFX(g.sfxJump, .08f);
        spawnParticles(g.player.x + (wallL ? -2.f : g.player.w), g.player.y + g.player.h*.4f, {180,230,255,255}, 10, 3.5f, .3f);
    } else if (jpr && !g.player.jumpHeld && !g.player.grounded && !g.player.doubleJumped && g.player.coyoteFrames <= 0) {
        g.player.vy = JUMP_FORCE * .9f; g.player.doubleJumped = true;
        if (g.audioReady) playSFX(g.sfxJump, .12f);
        spawnParticles(g.player.x + g.player.w/2, g.player.y + g.player.h, {100,200,255,255}, 8, 3.f, .3f);
    }
    if (g.player.jumpHeld && !jdn && g.player.vy < 0.f && g.player.dashTimer <= 0.f)
        g.player.vy *= JUMP_CUT_MULT;
    g.player.jumpHeld = jdn;
    if (prevGnd && !g.player.grounded) g.player.coyoteFrames = 6;
    else if (g.player.coyoteFrames > 0) g.player.coyoteFrames--;

    if (g.player.dashTimer <= 0.f)
        g.player.vy = std::min(g.player.vy + GRAVITY * DT60, 16.f);

    // Moving platforms
    for (auto& p : g.platforms) {
        if (p.vx != 0) { p.x += p.vx * DT60; if (p.x <= p.bound0 || p.x + p.w >= p.bound1) p.vx = -p.vx; }
        if (p.vy != 0) { p.y += p.vy * DT60; if (p.y <= p.bound0 || p.y + p.h >= p.bound1) p.vy = -p.vy; }
    }

    // Player movement and collision
    g.player.x = std::max(0.f, std::min(g.player.x + g.player.vx * DT60, g.levelWidth - g.player.w));
    g.player.y += g.player.vy * DT60;
    bool pg = g.player.grounded; g.player.grounded = false;
    for (const auto& p : g.platforms) {
        if (p.canVanish && p.vanishTimer >= 1.8f) continue; // phased out
        if (!overlaps(g.player.x, g.player.y, g.player.w, g.player.h, p.x, p.y, p.w, p.h)) continue;
        float ol = (g.player.x + g.player.w) - p.x;
        float or_ = (p.x + p.w) - g.player.x;
        float ot  = (g.player.y + g.player.h) - p.y;
        float ob  = (p.y + p.h) - g.player.y;
        float mn  = std::min({ol, or_, ot, ob});
        if (mn == ot && g.player.vy >= 0) {
            g.player.y = p.y - g.player.h; g.player.vy = 0; g.player.grounded = true;
            if (!pg) {
                spawnDust(g.player.x, g.player.y);
                g.player.squashTimer = 0.18f;
                if (g.audioReady && g.player.vy > 2.f) playSFX(g.sfxLand, .15f);
                g.player.dashCooldown = 0.f;
            }
        } else if (mn == ob && g.player.vy < 0) { g.player.y = p.y + p.h; g.player.vy = 0; }
        else if (mn == ol) g.player.x = p.x - g.player.w;
        else               g.player.x = p.x + p.w;
    }
    if (g.player.grounded) { g.player.doubleJumped = false; g.player.wallCooldown = 0.f; }

    // Vanishing platform timers
    for (auto& p : g.platforms) {
        if (!p.canVanish) continue;
        bool playerOn = g.player.grounded &&
                        (g.player.y + g.player.h) >= p.y - 2.f &&
                        (g.player.y + g.player.h) <= p.y + 6.f &&
                        g.player.x + g.player.w > p.x && g.player.x < p.x + p.w;
        if (playerOn)
            p.vanishTimer = std::min(p.vanishTimer + dt, 3.2f);
        else if (p.vanishTimer >= 1.8f)
            p.vanishTimer += dt;  // continue cycling through invisible phase
        if (p.vanishTimer >= 3.2f) p.vanishTimer = 0.f;  // reset cycle
    }

    // Ride moving platforms
    for (const auto& p : g.platforms)
        if ((p.vx || p.vy) && fabsf((g.player.y + g.player.h) - p.y) < 2.f
                && g.player.x + g.player.w > p.x && g.player.x < p.x + p.w)
            { g.player.x += p.vx * DT60; g.player.y += p.vy * DT60; }

    // Fell off world
    if (g.player.y > SH + 80) {
        g.levelDeaths++; g.totalDeaths++;
        if (g.audioReady) playSFX(g.sfxDie, .05f);
        shake(.5f, 10.f); g.flashTimer = 0.5f;
        g.combo = 0; g.comboTimer = 0.f;
        spawnParticles(g.player.x + g.player.w/2, SH/2, {231,76,60,255}, 20, 5.f, .8f);
        if (--g.lives <= 0) { g.screen = Screen::Over; return; }
        g.player.x = g.player.spawnX; g.player.y = g.player.spawnY;
        g.player.vx = 0; g.player.vy = 0;
        g.player.invFrames = (g.hardMode ? 45 : 90); g.player.powered = false;
        g.cameraX = std::max(0.f, g.player.x - SW/2.f); return;
    }

    // Crystal spikes — instant death (power-up absorbs one hit)
    for (const auto& s : g.spikes) {
        if (!overlaps(g.player.x + 3, g.player.y + g.player.h - 10, g.player.w - 6, 12,
                      s.x, s.y - 12, s.w, 14)) continue;
        if (g.player.invFrames > 0) continue;
        if (g.player.powered) {
            g.player.powered = false; g.player.invFrames = (g.hardMode ? 45 : 90);
            shake(.2f, 4.f); g.flashTimer = 0.25f;
            spawnPopup(g.player.x + 14, g.player.y, "SHIELD!", {100,200,255,255});
        } else {
            g.levelDeaths++; g.totalDeaths++;
            if (g.audioReady) playSFX(g.sfxDie, .05f);
            shake(.5f, 10.f); g.flashTimer = 0.5f;
            g.combo = 0; g.comboTimer = 0.f;
            spawnParticles(g.player.x + g.player.w/2, g.player.y + g.player.h/2, {231,76,60,255}, 20, 5.f, .8f);
            if (--g.lives <= 0) { g.screen = Screen::Over; return; }
            g.player.x = g.player.spawnX; g.player.y = g.player.spawnY;
            g.player.vx = 0; g.player.vy = 0;
            g.player.invFrames = (g.hardMode ? 45 : 90); g.player.powered = false;
            g.cameraX = std::max(0.f, g.player.x - SW/2.f); return;
        }
    }

    // Lava geysers — update timers and check player collision
    for (auto& geo : g.geysers) {
        geo.cooldown -= dt;
        if (!geo.firing && geo.cooldown <= 0.f) {
            geo.firing = true; geo.fireTimer = 0.7f;
        }
        if (geo.firing) {
            geo.fireTimer -= dt;
            if (geo.fireTimer <= 0.f) { geo.firing = false; geo.cooldown = geo.maxCooldown; }
            if (g.player.invFrames > 0) continue;
            // Column from (x-16, 300) to (x+16, 400) — above the gap
            if (!overlaps(g.player.x + 3, g.player.y + 3, g.player.w - 6, g.player.h - 6,
                          geo.x - 16.f, 300.f, 32.f, 110.f)) continue;
            if (g.player.powered) {
                g.player.powered = false; g.player.invFrames = (g.hardMode ? 45 : 90);
                shake(.2f, 4.f); g.flashTimer = 0.25f;
                spawnPopup(g.player.x + 14, g.player.y, "SHIELD!", {100,200,255,255});
            } else {
                g.levelDeaths++; g.totalDeaths++;
                if (g.audioReady) playSFX(g.sfxDie, .05f);
                shake(.5f, 10.f); g.flashTimer = 0.5f;
                g.combo = 0; g.comboTimer = 0.f;
                spawnParticles(g.player.x + g.player.w/2, g.player.y + g.player.h/2, {231,76,60,255}, 20, 5.f, .8f);
                if (--g.lives <= 0) { g.screen = Screen::Over; return; }
                g.player.x = g.player.spawnX; g.player.y = g.player.spawnY;
                g.player.vx = 0; g.player.vy = 0;
                g.player.invFrames = (g.hardMode ? 45 : 90); g.player.powered = false;
                g.cameraX = std::max(0.f, g.player.x - SW/2.f); return;
            }
        }
    }

    // Rising lava floor (stage 9)
    if (g.risingLavaY < 9999.f) {
        int diff = (g.currentStage - 1) * 10 + (g.levelInStage - 1);
        g.risingLavaY -= (0.18f + diff * 0.002f) * DT60;
        if (g.player.y + g.player.h >= g.risingLavaY && g.player.invFrames == 0) {
            g.levelDeaths++; g.totalDeaths++;
            if (g.audioReady) playSFX(g.sfxDie, .05f);
            shake(.5f, 10.f); g.flashTimer = 0.5f;
            g.combo = 0; g.comboTimer = 0.f;
            spawnParticles(g.player.x + g.player.w/2, g.player.y + g.player.h/2, {231,76,60,255}, 20, 5.f, .8f);
            if (--g.lives <= 0) { g.screen = Screen::Over; return; }
            // Respawn raises lava reset point — lava doesn't reset, player just respawns above it
            g.player.x = g.player.spawnX; g.player.y = g.player.spawnY;
            g.player.vx = 0; g.player.vy = 0;
            g.player.invFrames = (g.hardMode ? 45 : 90); g.player.powered = false;
            g.cameraX = std::max(0.f, g.player.x - SW/2.f);
            // Push lava back down a bit so player has a chance
            g.risingLavaY = std::max(g.risingLavaY, g.player.spawnY + 150.f); return;
        }
    }

    // Checkpoints
    for (auto& cp : g.checkpoints)
        if (!cp.activated && g.player.x + g.player.w > cp.x && g.player.x < cp.x + 12 && g.player.y + g.player.h > cp.y) {
            cp.activated = true; g.player.spawnX = cp.x; g.player.spawnY = cp.y - g.player.h;
            spawnParticles(cp.x, cp.y, {241,196,15,255}, 12, 3.f, .5f);
            spawnPopup(cp.x, cp.y - 20, "CHECKPOINT!", {39,174,96,255});
        }

    // Mushrooms
    for (auto& m : g.mushrooms) {
        if (m.collected) continue;
        if (overlaps(g.player.x, g.player.y, g.player.w, g.player.h, m.x-10, m.y-12, 20, 22)) {
            m.collected = true;
            if (!g.player.powered) { g.player.powered = true; unlockAch(2, "Super Mushroom"); }
            else g.player.invFrames = (g.hardMode ? 45 : 90);
            if (g.audioReady) playSFX(g.sfxPowerup, .08f);
            spawnParticles(m.x, m.y, {231,76,60,255}, 12, 4.f, .5f);
            g.score += 50;
            spawnPopup(m.x, m.y - 16, "POWER UP! +50", {241,196,15,255});
        }
    }

    // Coins
    for (auto& c : g.coins) {
        if (c.collected) continue;
        float dx = (g.player.x + g.player.w/2) - c.x, dy = (g.player.y + g.player.h/2) - c.y, r = g.player.w/2 + 8.f;
        if (dx*dx + dy*dy < r*r) {
            c.collected = true; g.score += 10;
            g.player.dashCooldown = 0.f;
            if (g.audioReady) playSFX(g.sfxCoin, .14f);
            spawnParticles(c.x, c.y, {241,196,15,255}, 6, 2.5f, .35f);
            spawnPopup(c.x, c.y - 12, "+10", {241,196,15,255});
            { bool allC = true; for (const auto& cc : g.coins) if (!cc.collected) { allC = false; break; }
              if (allC) unlockAch(5, "Coin Collector!"); }
        }
    }
    if (g.player.invFrames > 0) g.player.invFrames--;

    // Flag pole — active only when all bosses dead
    bool bossAlive = false;
    for (const auto& e : g.enemies) if (e.type == 3 && e.alive) bossAlive = true;
    float flagX = g.levelWidth - 80.f;
    if (!bossAlive && overlaps(g.player.x, g.player.y, g.player.w, g.player.h, flagX, 280, 20, 125)) {
        stopAllBGM();
        spawnParticles(g.player.x + g.player.w/2, g.player.y, {241,196,15,255}, 30, 5.f, 1.f);
        spawnParticles(g.player.x + g.player.w/2, g.player.y, {100,200,255,255}, 20, 4.f, .8f);
        // Best time tracked for full game run (stage 3 level 8 = final level)
        if (g.currentStage == 3 && g.levelInStage == 8) {
            if (g.bestTime == 0.f || g.elapsed < g.bestTime) g.bestTime = g.elapsed;
            if (g.elapsed < 3600.f) unlockAch(4, "Speed Runner (<60 min)");
        }
        finishLevel();
        return;
    }

    // Tick deathTimer for dying enemies
    for (auto& e : g.enemies) if (!e.alive && e.deathTimer > 0.f) e.deathTimer -= dt;

    // Enemy AI and player collision
    for (auto& e : g.enemies) {
        if (!e.alive) continue;
        if (e.flashTimer > 0.f) e.flashTimer = std::max(0.f, e.flashTimer - dt);

        if (e.type == 3) {
            float hm = (g.hardMode ? 1.35f : 1.f);
            if (g.currentStage == 1) {
                // Warden: phase 1 (HP=2) slow patrol + 1 shot/3s; phase 2 (HP=1) fast + 2 shots/2s
                bool p2 = (e.health == 1);
                float sp = (p2 ? 2.4f : 1.4f) * hm;
                e.vx = (e.vx >= 0 ? 1.f : -1.f) * sp;
                e.x += e.vx * DT60;
                if (e.x < e.x0 || e.x + 48 > e.x1) e.vx = -e.vx;
                e.shootTimer -= dt;
                if (e.shootTimer <= 0.f) {
                    e.shootTimer = p2 ? 2.f : 3.f;
                    float dir = (g.player.x > e.x ? 1.f : -1.f);
                    g.projectiles.push_back({e.x+24.f, e.y+26.f, dir * 3.f, true});
                    if (p2) g.projectiles.push_back({e.x+24.f, e.y+38.f, dir * 3.3f, true});
                    spawnParticles(e.x+24.f, e.y+30.f, {231,76,60,255}, p2 ? 6 : 4, 2.f, .2f);
                }
            } else if (g.currentStage == 2) {
                // Sentinel: phase 0 (HP=3) 2/2s; phase 1 (HP=2) 2/1.6s; phase 2 (HP=1) 3/1.2s
                int ph = (e.health == 1) ? 2 : (e.health == 2) ? 1 : 0;
                float sp = (1.8f + ph * 0.7f) * hm;
                e.vx = (e.vx >= 0 ? 1.f : -1.f) * sp;
                e.x += e.vx * DT60;
                if (e.x < e.x0 || e.x + 48 > e.x1) e.vx = -e.vx;
                e.shootTimer -= dt;
                if (e.shootTimer <= 0.f) {
                    float rates[] = {2.f, 1.6f, 1.2f};
                    e.shootTimer = rates[ph];
                    float dir = (g.player.x > e.x ? 1.f : -1.f);
                    int shots = (ph == 2) ? 3 : 2;
                    for (int si = 0; si < shots; si++)
                        g.projectiles.push_back({e.x+24.f, e.y+22.f+si*12.f, dir*(3.5f+si*0.3f), true});
                    spawnParticles(e.x+24.f, e.y+30.f, {231,76,60,255}, 4+ph*2, 3.f, .25f);
                }
            } else if (g.currentStage == 3) {
                // Inferno: fast patrol, rage at half HP, triple-shot, rising lava + BGM swell on rage
                bool rage = (e.health <= 2);
                if (rage && g.risingLavaY >= 9999.f) {
                    g.risingLavaY = 650.f;
                    spawnPopup(e.x+24.f, e.y-20.f, "RAGE!", {255,80,20,255});
                    shake(.6f, 14.f);
                    if (g.audioReady) SetMusicVolume(g.bgm[6], g.musicVolume * 1.3f);
                }
                float sp = (rage ? 2.8f : 2.0f + (4 - e.health) * 0.4f) * hm;
                e.vx = (e.vx >= 0 ? 1.f : -1.f) * sp;
                e.x += e.vx * DT60;
                if (e.x < e.x0 || e.x + 48 > e.x1) e.vx = -e.vx;
                e.shootTimer -= dt;
                if (e.shootTimer <= 0.f) {
                    e.shootTimer = rage ? 1.2f : 1.8f;
                    float dir = (g.player.x > e.x ? 1.f : -1.f);
                    int shots = rage ? 3 : 2;
                    for (int si = 0; si < shots; si++)
                        g.projectiles.push_back({e.x+24.f, e.y+22.f+si*14.f, dir*(3.5f+si*0.3f), true});
                    spawnParticles(e.x+24.f, e.y+30.f, {255,80,20,255}, 8, 3.f, .3f);
                }
            } else {
                // Generic fallback (stages 4+, unused in current build)
                int maxH = 2 + g.currentStage / 2;
                float sp = (1.2f + (maxH - std::min(e.health, maxH)) * .6f) * hm;
                e.vx = (e.vx >= 0 ? 1.f : -1.f) * sp;
                e.x += e.vx * DT60;
                if (e.x < e.x0 || e.x + 48 > e.x1) e.vx = -e.vx;
            }
        } else if (e.type == 1) {
            e.x += e.vx * DT60; e.y += e.vy * DT60;
            if (e.x < e.x0 || e.x + 28 > e.x1) e.vx = -e.vx;
            if (e.y < e.y0 || e.y + 28 > e.y1) e.vy = -e.vy;
        } else if (e.type == 2) {
            e.shootTimer -= dt;
            if (e.shootTimer <= 0) {
                e.shootTimer = 2.5f;
                g.projectiles.push_back({e.x+14, e.y+14, (g.player.x > e.x ? 3.5f : -3.5f), true});
                spawnParticles(e.x+14, e.y+14, {231,76,60,255}, 4, 2.f, .2f);
            }
        } else if (e.type == 4) {
            e.x += e.vx * DT60;
            if (e.x < e.x0 || e.x + 28 > e.x1) e.vx = -e.vx;
            e.vy = std::min(e.vy + GRAVITY * .85f * DT60, 14.f);
            e.y += e.vy * DT60;
            if (e.y >= e.y0) { e.y = e.y0; e.vy = 0.f; }
            e.shootTimer -= dt;
            if (e.vy == 0.f && e.shootTimer <= 0.f) {
                e.vy = JUMP_FORCE * .75f;
                e.shootTimer = 1.2f + GetRandomValue(0, 100) / 200.f;
            }
        } else {
            float hmult = g.hardMode ? 1.3f : 1.f;
            e.x += e.vx * hmult * DT60;
            if (e.x < e.x0 || e.x + 28 > e.x1) { e.vx = -e.vx; e.x += e.vx * DT60 * 2.f; }
        }

        float ew = (e.type == 3) ? 48.f : 28.f, eh = (e.type == 3) ? 60.f : 28.f;
        if (!overlaps(g.player.x, g.player.y, g.player.w, g.player.h, e.x, e.y, ew, eh)) continue;

        // Dash-kill: active dash kills on contact and refreshes dash
        if (g.player.dashTimer > 0.f) {
            if (e.type == 3) {
                e.health--;
                if (g.audioReady) playSFX(g.sfxBounce, .06f);
                shake(.3f, 6.f); g.hitStop = 6;
                spawnParticles(e.x+24, e.y, {231,76,60,255}, 10, 4.f, .4f);
                if (e.health <= 0) {
                    e.alive = false;
                    g.score += 500;
                    spawnPopup(e.x+24, e.y, "BOSS DOWN! +500", {241,196,15,255});
                    unlockAch(3, "Boss Slayer");
                    g.player.dashCooldown = 0.f;
                    bool anyBossLeft = false;
                    for (const auto& e2 : g.enemies) if (e2.type == 3 && e2.alive) anyBossLeft = true;
                    if (g.levelInStage == 8 && !anyBossLeft) {
                        stopAllBGM();
                        if (g.currentStage == 3) {
                            if (g.bestTime == 0.f || g.elapsed < g.bestTime) g.bestTime = g.elapsed;
                            if (g.elapsed < 3600.f) unlockAch(4, "Speed Runner (<60 min)");
                        }
                        finishLevel(); return;
                    }
                } else {
                    e.flashTimer = 2.f / 60.f;
                    int maxH = (g.currentStage <= 3) ? (1 + g.currentStage) : (2 + g.currentStage / 2);
                    if (e.health * 2 <= maxH) e.col = {255,60,20,255};
                    else                      e.col = {220,120,40,255};
                    if (g.currentStage == 1 && e.health == 1)
                        { spawnPopup(e.x+24, e.y-34, "PHASE 2!", {255,80,20,255}); shake(.6f, 14.f); }
                    else if (g.currentStage == 2 && e.health == 1)
                        { spawnPopup(e.x+24, e.y-34, "PHASE 3!", {255,60,10,255}); shake(.7f, 16.f); }
                    else if (g.currentStage == 2 && e.health == 2)
                        { spawnPopup(e.x+24, e.y-34, "PHASE 2!", {255,140,20,255}); shake(.4f, 10.f); }
                }
            } else {
                e.alive = false; e.deathTimer = 0.35f;
                if (g.audioReady) playSFX(g.sfxKill, .08f);
                chainKill(e.x+14, e.y+14, e.col);
                unlockAch(0, "First Stomp");
                if (g.combo >= 5) unlockAch(1, "Combo x5!");
                g.player.dashCooldown = 0.f;
            }
            continue;
        }

        bool stomped = (g.player.vy > 0 && (g.player.y + g.player.h) - e.y < (e.type == 3 ? 22 : 16));
        if (stomped) {
            g.player.vy = -9.f;
            if (e.type == 3) {
                e.health--; if (g.audioReady) playSFX(g.sfxBounce, .06f); shake(.3f, 6.f);
                g.hitStop = 6;
                spawnParticles(e.x+24, e.y, {231,76,60,255}, 10, 4.f, .4f);
                spawnPopup(e.x+24, e.y-10, "HIT!", {255,120,50,255});
                if (e.health <= 0) {
                    e.alive = false; g.score += 500; shake(.7f, 14.f); g.hitStop = 12;
                    spawnParticles(e.x+24, e.y+30, {231,76,60,255}, 30, 6.f, .8f);
                    spawnParticles(e.x+24, e.y+30, {241,196,15,255}, 20, 5.f, .6f);
                    spawnPopup(e.x+24, e.y, "BOSS DOWN! +500", {241,196,15,255});
                    unlockAch(3, "Boss Slayer");
                    bool anyBossLeft = false;
                    for (const auto& e2 : g.enemies) if (e2.type == 3 && e2.alive) anyBossLeft = true;
                    if (g.levelInStage == 8 && !anyBossLeft) {
                        stopAllBGM();
                        if (g.currentStage == 3) {
                            if (g.bestTime == 0.f || g.elapsed < g.bestTime) g.bestTime = g.elapsed;
                            if (g.elapsed < 3600.f) unlockAch(4, "Speed Runner (<60 min)");
                        }
                        finishLevel(); return;
                    }
                } else {
                    e.flashTimer = 2.f / 60.f;
                    int maxH = (g.currentStage <= 3) ? (1 + g.currentStage) : (2 + g.currentStage / 2);
                    if (e.health * 2 <= maxH) e.col = {255,60,20,255};
                    else                      e.col = {220,120,40,255};
                    if (g.currentStage == 1 && e.health == 1)
                        { spawnPopup(e.x+24, e.y-34, "PHASE 2!", {255,80,20,255}); shake(.6f, 14.f); }
                    else if (g.currentStage == 2 && e.health == 1)
                        { spawnPopup(e.x+24, e.y-34, "PHASE 3!", {255,60,10,255}); shake(.7f, 16.f); }
                    else if (g.currentStage == 2 && e.health == 2)
                        { spawnPopup(e.x+24, e.y-34, "PHASE 2!", {255,140,20,255}); shake(.4f, 10.f); }
                }
            } else {
                e.alive = false; e.deathTimer = 0.35f;
                if (g.audioReady) playSFX(g.sfxKill, .08f);
                chainKill(e.x+14, e.y+14, e.col);
                unlockAch(0, "First Stomp");
                if (g.combo >= 5) unlockAch(1, "Combo x5!");
            }
        } else if (g.player.invFrames == 0) {
            if (g.player.powered) {
                g.player.powered = false; g.player.invFrames = (g.hardMode ? 45 : 90);
                shake(.2f, 4.f); g.flashTimer = 0.25f;
                g.combo = 0; g.comboTimer = 0.f;
                spawnPopup(g.player.x+14, g.player.y, "SHIELD!", {100,200,255,255});
            } else {
                g.levelDeaths++; g.totalDeaths++;
                if (--g.lives <= 0) { g.screen = Screen::Over; return; }
                g.player.invFrames = (g.hardMode ? 45 : 90); shake(.35f, 6.f);
                g.flashTimer = 0.4f; g.combo = 0; g.comboTimer = 0.f;
                if (g.audioReady) playSFX(g.sfxDie, .05f);
                spawnParticles(g.player.x + g.player.w/2, g.player.y + g.player.h/2, {231,76,60,255}, 12, 4.f, .5f);
            }
        }
    }

    // Projectiles
    for (auto& p : g.projectiles) {
        if (!p.active) continue;
        p.x += p.vx * DT60;
        if (p.x < g.cameraX - 60 || p.x > g.cameraX + SW + 60) p.active = false;
        if (overlaps(g.player.x, g.player.y, g.player.w, g.player.h, p.x-5, p.y-5, 10, 10)
                && g.player.invFrames == 0 && g.player.dashTimer <= 0.f) {
            p.active = false;
            if (g.player.powered) {
                g.player.powered = false; g.player.invFrames = (g.hardMode ? 45 : 90);
                spawnPopup(g.player.x+14, g.player.y, "SHIELD!", {100,200,255,255});
            } else {
                g.levelDeaths++; g.totalDeaths++;
                if (--g.lives <= 0) { g.screen = Screen::Over; return; }
                g.player.invFrames = (g.hardMode ? 45 : 90); shake(.2f, 4.f);
                g.combo = 0; g.comboTimer = 0.f; g.flashTimer = 0.3f;
                if (g.audioReady) playSFX(g.sfxDie, .05f);
            }
        }
    }
    g.projectiles.erase(
        std::remove_if(g.projectiles.begin(), g.projectiles.end(), [](const Projectile& p) { return !p.active; }),
        g.projectiles.end());

    // Camera: lookahead smooth-damps toward velocity; camera decays to that target
    float lookaheadTarget = g.player.vx * CAM_LOOKAHEAD_VX + g.player.vy * CAM_LOOKAHEAD_VY;
    g.cameraLookahead += (lookaheadTarget - g.cameraLookahead) * (1.f - powf(0.82f, DT60));
    float camTarget = g.player.x - SW/2.f + g.player.w/2.f + g.cameraLookahead;
    g.cameraX += (camTarget - g.cameraX) * (1.f - powf(0.88f, DT60));
    g.cameraX = std::max(0.f, std::min(g.cameraX, g.levelWidth - (float)SW));

    // Boss arena BGM: switch to epic track (bgm[6]) when player crosses arena threshold
    if (g.levelInStage == 8 && g.player.x > 880.f && g.audioReady
            && !IsMusicStreamPlaying(g.bgm[6])) {
        bool bossAlive = false;
        for (const auto& e : g.enemies) if (e.type == 3 && e.alive) { bossAlive = true; break; }
        if (bossAlive) {
            stopAllBGM();
            SetMusicVolume(g.bgm[6], g.musicVolume);
            PlayMusicStream(g.bgm[6]);
        }
    }

    // Tutorial triggers (stage 1, level 1 only)
    if (g.currentStage == 1 && g.levelInStage == 1) {
        if (!(g.tutorialMask &  1) && g.player.x >  60) g.tutorialMask |=  1;
        if (!(g.tutorialMask &  2) && g.player.x > 450) g.tutorialMask |=  2;
        if (!(g.tutorialMask &  4) && g.player.x > 850) g.tutorialMask |=  4;
        if (!(g.tutorialMask &  8) && g.player.x >1400) g.tutorialMask |=  8;
        if (!(g.tutorialMask & 16) && g.player.x >1900) g.tutorialMask |= 16;
    }

    // Particles
    for (auto& p : g.particles) {
        p.x += p.vx * DT60; p.y += p.vy * DT60; p.vy += .15f * DT60;
        p.life -= dt;
        p.col.a = (unsigned char)(255 * (p.life / p.maxLife));
    }
    g.particles.erase(
        std::remove_if(g.particles.begin(), g.particles.end(), [](const Particle& p) { return p.life <= 0; }),
        g.particles.end());

    // Score popups
    for (auto& p : g.popups) {
        p.y += p.vy * DT60;
        p.vy = std::max(p.vy + 0.04f * DT60, -0.5f);
        p.life -= dt;
    }
    g.popups.erase(
        std::remove_if(g.popups.begin(), g.popups.end(), [](const Popup& p) { return p.life <= 0; }),
        g.popups.end());
}
