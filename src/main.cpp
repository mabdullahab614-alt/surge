#include "types.h"
#include "globals.h"
#include "audio.h"
#include "render.h"
#include "game.h"
#include "levels.h"
#include "Tuning.h"
#include <vector>
#include <algorithm>
#include <fstream>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

// ── Single global game state instance ─────────────────────────────────────────
GameState g;

// ── Module-level so gameFrame() can access them ───────────────────────────────
static RenderTexture2D gRt;
static float gAccumulator = 0.f;

// ── Persistence ───────────────────────────────────────────────────────────────
static void loadSettings(){
    std::ifstream f("settings.txt");
    if(!f) return;
    std::string tag; int ver = 1;
    if(f.peek() == 'V'){ f >> tag >> ver; }
    if(ver > 2){ f.close(); std::rename("settings.txt", "settings.txt.bak"); return; }
    f >> g.masterVolume >> g.musicVolume >> g.sfxVolume >> g.hardMode;
    if (ver >= 3) f >> g.keyLeft >> g.keyRight >> g.keyJump >> g.keyDash;
}
void saveSettings(){
    std::ofstream f("settings.txt");
    f << "VERSION 3\n";
    f << g.masterVolume << " " << g.musicVolume << " " << g.sfxVolume << " " << g.hardMode
      << " " << g.keyLeft << " " << g.keyRight << " " << g.keyJump << " " << g.keyDash;
}
static void loadRecords(){
    std::ifstream f("records.txt");
    if(!f) return;
    std::string tag; int ver = 1;
    if(f.peek() == 'V'){ f >> tag >> ver; }
    if(ver > 2){ f.close(); std::rename("records.txt", "records.txt.bak"); return; }
    f >> g.highScore >> g.bestTime
      >> g.levelBestTimes[1] >> g.levelBestTimes[2]
      >> g.levelBestTimes[3] >> g.levelBestTimes[4]
      >> g.levelBestTimes[5] >> g.levelBestTimes[6]
      >> g.achievementFlags;
    for(int i = 1; i <= 10; i++) f >> g.stageProgress[i];
    for(int s = 1; s <= 10; s++) for(int l = 1; l <= 10; l++) {
        if(!(f >> g.levelGrades[s][l])) break;
    }
    f >> g.totalDeaths;
}
void saveRecords(){
    std::ofstream f("records.txt");
    f << "VERSION 2\n";
    f << g.highScore   << " " << g.bestTime         << " "
      << g.levelBestTimes[1] << " " << g.levelBestTimes[2] << " "
      << g.levelBestTimes[3] << " " << g.levelBestTimes[4] << " "
      << g.levelBestTimes[5] << " " << g.levelBestTimes[6] << " "
      << g.achievementFlags;
    for(int i = 1; i <= 10; i++) f << " " << g.stageProgress[i];
    for(int s = 1; s <= 10; s++) for(int l = 1; l <= 10; l++) f << " " << g.levelGrades[s][l];
    f << " " << g.totalDeaths;
}
static void shutdownAudio(){
    UnloadSound(g.sfxJump);   UnloadSound(g.sfxCoin);
    UnloadSound(g.sfxStomp);  UnloadSound(g.sfxDie);
    UnloadSound(g.sfxPowerup); UnloadSound(g.sfxBounce);
    UnloadSound(g.sfxLand);   UnloadSound(g.sfxDash);
    UnloadSound(g.sfxKill);
    UnloadSound(g.sfxStageWin);
    for(int i = 1; i <= 6; i++) UnloadMusicStream(g.bgm[i]);
    CloseAudioDevice();
}
void applyVolumes(){
    SetMasterVolume(g.masterVolume);
    if(g.audioReady){
        SetMusicVolume(g.bgm[1], g.musicVolume);
        SetMusicVolume(g.bgm[2], g.musicVolume);
        SetMusicVolume(g.bgm[3], g.musicVolume * 1.05f);
        SetMusicVolume(g.bgm[4], g.musicVolume);
        SetMusicVolume(g.bgm[5], g.musicVolume);
        SetMusicVolume(g.bgm[6], g.musicVolume * 1.1f);
    }
}

// ── One frame of game logic + rendering ───────────────────────────────────────
static void gameFrame(){
    float rawDt = std::min(GetFrameTime(), 0.05f);
    g.menuTimer += rawDt;

    for(auto& a : g.achievementNotifs) a.life -= rawDt;
    g.achievementNotifs.erase(
        std::remove_if(g.achievementNotifs.begin(), g.achievementNotifs.end(),
            [](const AchNotif& a){ return a.life <= 0; }),
        g.achievementNotifs.end());

    if(IsKeyPressed(KEY_F11)) ToggleFullscreen();
    if(IsKeyPressed(KEY_F2)){
        if(g.screen != Screen::Edit && g.screen != Screen::Menu && g.screen != Screen::Settings){
            g.screenBeforeEdit = g.screen;
            g.screen           = Screen::Edit;
            g.editorCameraX    = g.cameraX;
            pauseAllBGM();
        } else if(g.screen == Screen::Edit){
            g.screen    = g.screenBeforeEdit;
            g.cameraX   = g.editorCameraX;
            resumeAllBGM();
        }
    }

    if(g.rebindTarget > 0) {
        int k = GetKeyPressed();
        if(k == KEY_ESCAPE) { g.rebindTarget = 0; }
        else if(k > 0) {
            if     (g.rebindTarget == 1) g.keyLeft  = k;
            else if(g.rebindTarget == 2) g.keyRight = k;
            else if(g.rebindTarget == 3) g.keyJump  = k;
            else if(g.rebindTarget == 4) g.keyDash  = k;
            g.rebindTarget = 0;
            saveSettings();
        }
    }

    bool gpConn = IsGamepadAvailable(0);
    if(g.rebindTarget == 0 && (IsKeyPressed(KEY_ESCAPE) || (gpConn && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)))){
        if     (g.screen == Screen::Edit)     { g.screen = g.screenBeforeEdit; g.cameraX = g.editorCameraX; resumeAllBGM(); }
        else if(g.screen == Screen::Settings) {
            g.screen = (g.screenBeforeEdit == Screen::Pause) ? Screen::Pause : Screen::Menu;
            saveSettings(); applyVolumes();
        }
        else if(g.screen == Screen::Play)     { g.screen = Screen::Pause; pauseAllBGM(); }
        else if(g.screen == Screen::Pause)    { g.screen = Screen::Play;  resumeAllBGM(); }
    }
    if((IsKeyPressed(KEY_R) || (gpConn && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)))
        && (g.screen == Screen::Over || g.screen == Screen::Win || g.screen == Screen::GameWinner))
        resetGame();

    if(g.screen == Screen::Settings) applyVolumes();

    if(g.screen == Screen::LevelComplete){
        g.levelCompleteTimer += rawDt;
        if(g.levelCompleteTimer > 3.5f){
            g.levelCompleteTimer = 0.f;
            if(g.currentStage == 3 && g.levelInStage == 8){
                g.gameWinTimer = 0.f;
                g.screen = Screen::GameWinner;
                if(g.audioReady) playSFX(g.sfxStageWin, 0.f);
            } else if(g.levelInStage == 8){
                g.stageCompleteTimer = 0.f;
                g.screen = Screen::StageComplete;
                if(g.audioReady) playSFX(g.sfxStageWin, 0.f);
            } else {
                g.levelInStage++;
                g.stageMapTimer = 0.f;
                g.screen = Screen::StageMap;
            }
        }
    }
    if(g.screen == Screen::StageMap){
        g.stageMapTimer += rawDt;
        if(g.stageMapTimer > 2.5f){
            g.stageMapTimer = 0.f;
            initLevel();
            playLevelBGM();
            g.fadeAlpha = 1.f;
            g.screen = Screen::Play;
        }
    }
    if(g.screen == Screen::StageComplete){
        g.stageCompleteTimer += rawDt;
        if(g.stageCompleteTimer > 3.5f){
            g.stageCompleteTimer = 0.f;
            g.currentStage++;
            g.levelInStage = 1;
            g.stageMapTimer = 0.f;
            g.screen = Screen::StageMap;
        }
    }
    if(g.screen == Screen::GameWinner){
        g.gameWinTimer += rawDt;
    }

    if(g.screen == Screen::Play && g.currentStage == 1 && g.levelInStage == 1 && !(g.tutorialMask & 0x80)) {
        if(GetKeyPressed() != 0 || (gpConn && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)))
            g.tutorialMask |= 0x80;
        if(g.elapsed - g.levelStartTime > 4.f) g.tutorialMask |= 0x80;
    }

    if(g.screen == Screen::Play){
        if(g.timeScaleTimer > 0.f){
            g.timeScaleTimer -= rawDt;
            if(g.timeScaleTimer <= 0.f){ g.timeScaleTimer = 0.f; g.timeScale = 1.f; }
        }
        gAccumulator += rawDt * g.timeScale;
        while(gAccumulator >= FIXED_DT){
            update(FIXED_DT);
            gAccumulator -= FIXED_DT;
        }
        if(g.fadeAlpha > 0.f) g.fadeAlpha = std::max(0.f, g.fadeAlpha - rawDt * 2.5f);
    }

    if(g.audioReady) updateActiveBGM();

    float scaleX = (float)GetScreenWidth()  / (float)SW;
    float scaleY = (float)GetScreenHeight() / (float)SH;
    float scale  = std::min(scaleX, scaleY);
    float destW  = (float)SW * scale, destH = (float)SH * scale;
    float offX   = ((float)GetScreenWidth()  - destW) * 0.5f;
    float offY   = ((float)GetScreenHeight() - destH) * 0.5f;
    Vector2 rawMp = GetMousePosition();
    g.mouse = { (rawMp.x - offX) / scale, (rawMp.y - offY) / scale };

    BeginTextureMode(gRt);
        ClearBackground(BLACK);
        if     (g.screen == Screen::Menu)     drawMenu();
        else if(g.screen == Screen::Settings) drawSettings();
        else if(g.screen == Screen::Edit)     drawEditor();
        else                                  drawScene();
    EndTextureMode();

    BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(gRt.texture,
            {0.f, 0.f, (float)SW, -(float)SH},
            {offX, offY, destW, destH},
            {0.f, 0.f}, 0.f, WHITE);
    EndDrawing();
}

// ── Main ──────────────────────────────────────────────────────────────────────
int main(){
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SW, SH, "SURGE \xe2\x80\x94 by Abdullah Javid");
    initSprites();
    gRt = LoadRenderTexture(SW, SH);
    SetTextureFilter(gRt.texture, TEXTURE_FILTER_POINT);
    loadSettings();
    loadRecords();

    if(InitAudioDevice(), IsAudioDeviceReady()){
        g.audioReady  = true;
        g.sfxJump     = mkSweep(300,  800, .15f);
        g.sfxCoin     = mkTone (1047,      .12f);
        g.sfxStomp    = mkSweep(300,   80, .2f);
        g.sfxDie      = mkSweep(600,  120, .5f,  16000);
        g.sfxPowerup  = mkSweep(440,  880, .25f);
        g.sfxBounce   = mkSweep(200,  400, .15f, 20000);
        g.sfxLand     = mkSweep(220,   80, .08f, 10000);
        g.sfxDash     = mkSweep(600, 2200, .1f,  13000);
        g.sfxKill     = mkCrunch();
        g.sfxStageWin = mkStageWin();
        g.bgm[1] = mkBGM();
        g.bgm[2] = mkBGM2();
        g.bgm[3] = mkBGM3();
        g.bgm[4] = mkBGM4();
        g.bgm[5] = mkBGM5();
        g.bgm[6] = mkBGM6();
        applyVolumes();
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(gameFrame, 0, 1);
#else
    while(!WindowShouldClose()) gameFrame();

    saveSettings();
    saveRecords();
    if(g.audioReady) shutdownAudio();
    deinitSprites();
    UnloadRenderTexture(gRt);
    CloseWindow();
#endif
    return 0;
}
