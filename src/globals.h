#pragma once
#include "types.h"
#include "raylib.h"
#include <vector>

struct GameState {
    // Score and progression
    int   score        = 0;
    int   lives        = 3;
    int   highScore    = 0;
    int   currentLevel = 1;
    float elapsed      = 0.f;
    float bestTime     = 0.f;
    float menuTimer    = 0.f;

    // Camera and visual effects
    float cameraX            = 0.f;
    float shakeTimer         = 0.f;
    float shakeAmount        = 0.f;
    float shakeDecay         = 1.f;
    float levelCompleteTimer = 0.f;
    float flashTimer         = 0.f;
    float fadeAlpha          = 0.f;
    float levelWidth         = 3200.f;

    // Combat and combo
    int   combo      = 0;
    float comboTimer = 0.f;
    int   maxCombo   = 0;
    int   hitStop    = 0;
    float timeScale       = 1.f;
    float timeScaleTimer  = 0.f;
    float vignettePulse   = 0.f;
    float cameraLookahead = 0.f;
    Vector2 mouse         = {};

    // State machine
    Screen screen           = Screen::Menu;
    Screen screenBeforeEdit = Screen::Menu;

    // Player
    Player player;

    // World objects
    std::vector<Platform>   platforms;
    std::vector<Coin>       coins;
    std::vector<Enemy>      enemies;
    std::vector<Mushroom>   mushrooms;
    std::vector<Checkpoint> checkpoints;
    std::vector<Projectile> projectiles;
    std::vector<Particle>   particles;
    std::vector<Popup>      popups;
    std::vector<Spike>      spikes;
    std::vector<Geyser>     geysers;

    // Settings
    float masterVolume = 1.f;
    float musicVolume  = 0.4f;
    float sfxVolume    = 0.85f;
    bool  hardMode     = false;
    bool  audioReady   = false;

    // Key bindings (rebindable)
    int keyLeft  = 263;  // KEY_LEFT
    int keyRight = 262;  // KEY_RIGHT
    int keyJump  = 32;   // KEY_SPACE
    int keyDash  = 340;  // KEY_LEFT_SHIFT
    int rebindTarget = 0; // 0=none 1=left 2=right 3=jump 4=dash

    // Stage / level progression (10 stages × 10 levels)
    int   currentStage   = 1;   // 1-10
    int   levelInStage   = 1;   // 1-10
    int   stageProgress[11] = {};    // levels cleared per stage, index 1-10
    float stageBestTimes[11] = {};   // best time per stage, index 1-10
    float stageMapTimer      = 0.f;  // auto-advance timer on StageMap screen
    float stageCompleteTimer = 0.f;
    float gameWinTimer       = 0.f;

    // Per-level stats and grading
    int   levelDeaths    = 0;   // deaths in current level run
    int   totalDeaths    = 0;   // deaths across entire game session
    float levelStartTime = 0.f; // g.elapsed when current level began
    float windForce      = 0.f; // horizontal wind acceleration per frame
    float risingLavaY    = 9999.f; // Y of rising lava floor (9999 = inactive)

    // Grade storage: 0=unplayed, 1=C, 2=B, 3=A, 4=S
    int levelGrades[11][11] = {};

    // Scorecard display values (set when a level finishes)
    int   lastGrade      = 0;
    int   lastDeaths     = 0;
    float lastLevelTime  = 0.f;
    float lastCoinPct    = 0.f;
    int   lastCoinsGot   = 0;
    int   lastCoinsTotal = 0;
    int   lastMaxCombo   = 0;

    // Progress
    int   achievementFlags = 0;
    std::vector<AchNotif> achievementNotifs;
    int   tutorialMask = 0;
    float levelBestTimes[7] = {};

    // Level editor
    int   editorTool    = 0;
    float editorCameraX = 0.f;

    // SFX handles
    Sound sfxJump    = {};
    Sound sfxCoin    = {};
    Sound sfxStomp   = {};
    Sound sfxDie     = {};
    Sound sfxPowerup = {};
    Sound sfxBounce  = {};
    Sound sfxLand    = {};
    Sound sfxDash    = {};
    Sound sfxKill     = {};
    Sound sfxStageWin = {};   // ascending stinger on stage/game clear

    float bgmDipTimer = 0.f;  // seconds remaining in chain-x10 BGM volume dip

    // BGM handles: bgm[1..6] for levels 1-6
    Music bgm[7] = {};
};

extern GameState g;

// Persistence helpers (defined in main.cpp)
void saveSettings();
void saveRecords();
void applyVolumes();
