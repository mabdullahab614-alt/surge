#pragma once
#include "raylib.h"
#include <vector>

static const int   SW = 800, SH = 450;
static const float GRAVITY    = 0.52f;
static const float JUMP_FORCE = -12.f;
static const float MOVE_SPEED = 4.f;
static const float LEVEL_WIDTHS[] = {0.f, 3200.f, 3800.f, 4400.f, 5000.f, 5600.f, 6200.f};

struct Platform   { float x, y, w, h, vx=0, vy=0, bound0=0, bound1=0; float vanishTimer=0.f; bool canVanish=false; };
struct Spike      { float x, y, w; };
struct Geyser     { float x, cooldown, maxCooldown; bool firing; float fireTimer; };
struct Coin       { float x, y; bool collected; };
struct Mushroom   { float x, y; bool collected; };
struct Checkpoint { float x, y; bool activated; };
struct Projectile { float x, y, vx; bool active; };
struct Cloud      { float cx, cy, cw, ch; };
struct Particle   { float x, y, vx, vy, life, maxLife, size; Color col; };
struct Popup      { float x, y, vy, life, maxLife; char text[24]; Color col; };
struct AchNotif   { char name[48]; float life; };

struct Enemy {
    float x, y, vx, vy, x0, x1, y0, y1;
    bool  alive;
    Color col;
    int   type;
    float shootTimer;
    int   health;
    float deathTimer  = 0.f;
    float flashTimer  = 0.f;
};

struct Player {
    float x=50, y=350, vx=0, vy=0, w=28, h=36;
    bool  grounded=false, jumpHeld=false;
    int   facing=1, invFrames=0, walkFrame=0, walkTimer=0;
    int   coyoteFrames=0, jumpBuffer=0;
    bool  doubleJumped=false, powered=false;
    float spawnX=50, spawnY=350;
    float squashTimer=0.f;
    float dashTimer=0.f, dashCooldown=0.f;
    float dashDirX=1.f, dashDirY=0.f;
    float aiX[5]={}, aiY[5]={};
    int   aiCount=0;
    float wallCooldown=0.f;
};

enum class Screen { Menu, Play, Pause, LevelComplete, Over, Win, Edit, Settings,
                    StageMap, StageComplete, GameWinner };

static const Cloud CLOUDS[] = {
    {120,70,75,32},{330,55,90,35},{590,85,65,28},{870,65,80,30},{1150,80,70,28}
};
