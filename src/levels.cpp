#include "levels.h"
#include "globals.h"
#include "procgen.h"

// ── Placement helpers ─────────────────────────────────────────────────────────
static const float GY  = 400.f;
// Per-stage enemy palette
static const Color EC1 = {255,   0, 110, 255};  // Grasslands — hot pink
static const Color EC2 = {  0, 220, 200, 255};  // Crystal Caves — teal
static const Color EC3 = {255, 100,  20, 255};  // Volcano Peak — ember orange

static void gnd  (float x, float w) { g.platforms.push_back({x,GY,w,50.f}); }
static void flt  (float x, float y, float w,
                  float vx=0,float vy=0,float b0=0,float b1=0) {
    g.platforms.push_back({x,y,w,18.f,vx,vy,b0,b1});
}
static void fltv (float x, float y, float w) {   // vanishing floating platform
    flt(x,y,w); g.platforms.back().canVanish = true;
}
static void cns  (float x, float y, int n=3, float gap=22.f) {
    for (int i=0;i<n;i++) g.coins.push_back({x+i*gap, y, false});
}
static void chsr (float x, float x0, float x1, float spd=1.7f, Color c=EC1) {
    g.enemies.push_back({x,GY-28.f,spd,0,x0,x1,0,0,true,c,0,0.f,0});
}
static void shtr (float x, float x0, float x1, float ival=2.3f, Color c=EC1) {
    g.enemies.push_back({x,GY-28.f,0,0,x0,x1,0,0,true,c,2,ival,0});
}
static void flyr (float x, float y, float x0, float x1,
                  float y0, float y1, float spd=1.4f, Color c=EC1) {
    g.enemies.push_back({x,y,spd,0.9f,x0,x1,y0,y1,true,c,1,0.f,0});
}
static void jmpr (float x, float x0, float x1, float spd=1.3f, Color c=EC1) {
    g.enemies.push_back({x,GY-28.f,spd,0,x0,x1,GY-28.f,0,true,c,4,0.9f,0});
}
static void bss  (float x, float x0, float x1, int hp, float spd=1.5f, Color c=EC1) {
    g.enemies.push_back({x,GY-60.f,spd,0,x0,x1,0,0,true,c,3,2.0f,hp});
}
static void mush (float x, float py=GY) { g.mushrooms.push_back({x,py-28.f,false}); }
static void chkpt(float x, float py=GY) { g.checkpoints.push_back({x,py-28.f,false}); }
static void spk  (float x, float w)     { g.spikes.push_back({x,GY,w}); }
static void geysr(float x, float cd=3.f){ g.geysers.push_back({x,cd,cd,false,0.f}); }

// ── Stage 1: Grasslands  (theme 1 · bgm[1]) ──────────────────────────────────

static void s1l1() {
    // First Steps — wide platforms, tiny gaps, learn dash, 2 light chasers
    g.levelWidth = 2400.f;
    gnd(0,280); gnd(360,200); gnd(640,200); gnd(920,220);
    gnd(1200,200); gnd(1480,220); gnd(1760,220); gnd(2100,300);
    flt(660,330,140); flt(1220,310,130); flt(1790,320,120);
    cns(670, 308);    cns(950,  GY-22.f, 2);
    cns(1230,288);    cns(1510, GY-22.f, 2);
    cns(1800,298, 2); cns(2130, GY-22.f, 2);
    chsr(1260,1200,1400);
    chsr(1800,1760,1950);
    mush(970); chkpt(1100);
}

static void s1l2() {
    // Combo Lane — first chain-kill: 3 chasers within dash reach of each other
    g.levelWidth = 2600.f;
    gnd(0,280); gnd(380,220); gnd(660,240); gnd(980,320);
    gnd(1380,260); gnd(1720,240); gnd(2300,300);
    flt(700,320,110); flt(1040,340,110); flt(1760,310,120);
    cns(380+20, GY-22.f, 2);
    cns(710,    298,      3);
    cns(1020,   GY-22.f, 4, 22.f); // 4 coins through the chain-kill zone
    cns(1050,   318,      2);
    cns(1740,   GY-22.f, 2);
    cns(1780,   288,      3);
    chsr(420,  380, 580,  1.5f);
    chsr(1020, 980,1140,  1.6f); // chain cluster
    chsr(1130, 980,1300,  1.6f);
    chsr(1250,1100,1360,  1.6f);
    flyr(1800,295, 1740,1980, 270,320);
    mush(1200); chkpt(940);
}

static void s1l3() {
    // Up and Over — moving platforms, jumpers, more vertical
    g.levelWidth = 2600.f;
    gnd(0,260); gnd(380,160); gnd(620,200); gnd(900,200);
    gnd(1160,200); gnd(1440,200); gnd(1700,200); gnd(2300,300);
    flt(400,315,130);
    flt(780,300,120, 1.2f,0, 700,900);
    flt(960,280,110);
    flt(1220,330,120, 1.4f,0, 1160,1400);
    flt(1520,310,110); flt(1760,290,120);
    cns(410, 293); cns(650, GY-22.f, 2);
    cns(980, 258); cns(1180,GY-22.f, 2);
    cns(1530,288); cns(1770,268);
    chsr(650,  620, 800,  1.8f);
    jmpr(980,  900,1080);
    chsr(1200,1160,1340,  1.8f);
    jmpr(1540,1440,1640);
    flyr(1770,275, 1700,1900, 255,300);
    mush(1080); chkpt(900);
}

static void s1l4() {
    // Gauntlet — dense enemies, first 2 shooters, pre-boss pressure
    g.levelWidth = 2800.f;
    gnd(0,260); gnd(360,200); gnd(640,200); gnd(920,240);
    gnd(1240,200); gnd(1520,240); gnd(1800,200); gnd(2500,300);
    flt(380,310,100); flt(660,320,110);
    flt(960,300,120); flt(1560,310,110);
    flt(1820,300,120); flt(2060,320,100);
    cns(385, 288);    cns(665, 298);
    cns(960, GY-22.f,4); // chain-kill reward
    cns(970, 278, 2);
    cns(1560,GY-22.f,2); cns(1570,288, 2);
    cns(2070,298);
    chsr(400,  360, 540,  1.8f); chsr(680,  640, 820,  1.8f);
    chsr(960,  920,1100,  1.8f); chsr(1060, 920,1220,  1.8f);
    shtr(1280,1240,1460,  2.2f);
    jmpr(1570,1520,1700);
    chsr(1840,1800,1980,  2.0f);
    shtr(2080,2020,2260,  2.4f);
    flyr(2200,290, 2140,2460, 265,315);
    mush(1350); chkpt(860);
}

// ── Stage 2: Crystal Caves  (theme 3 · bgm[3]) ───────────────────────────────

static void s2l1() {
    // Crystal Grotto — spikes intro, 4 teal chasers
    g.levelWidth = 2400.f;
    gnd(0,260); gnd(360,180); gnd(620,180); gnd(880,200);
    gnd(1140,200); gnd(1400,200); gnd(1660,200); gnd(2100,300);
    flt(380,320,120); flt(900,310,110); flt(1420,305,120);
    spk(600,28); spk(1120,24); spk(1640,28);
    cns(390,298); cns(920, GY-22.f,2);
    cns(1430,283); cns(1680,GY-22.f,2);
    cns(2130,GY-22.f,2);
    chsr(400, 360, 520, 1.8f,EC2); chsr(660, 620, 780, 1.8f,EC2);
    chsr(940, 880,1060, 1.8f,EC2); chsr(1450,1400,1560, 1.8f,EC2);
    mush(1200); chkpt(860);
}

static void s2l2() {
    // Diamond Hall — flyers over gaps, moving platforms
    g.levelWidth = 2600.f;
    gnd(0,260); gnd(380,180); gnd(660,180); gnd(940,200);
    gnd(1220,200); gnd(1500,200); gnd(1780,200); gnd(2300,300);
    flt(660,320,110, 1.3f,0, 580,800);
    flt(1000,310,100);
    flt(1280,330,110, 1.4f,0, 1200,1460);
    flt(1800,305,110);
    spk(620,24); spk(900,24); spk(1460,28); spk(1760,24);
    cns(400, GY-22.f, 2); cns(1010,288, 2);
    cns(1300,GY-22.f, 2); cns(1810,283, 2);
    cns(2330,GY-22.f, 2);
    flyr(500, 290, 380, 660, 265,320, 1.4f,EC2);
    flyr(820, 285, 660, 940, 260,315, 1.4f,EC2);
    flyr(1380,280,1220,1500, 255,310, 1.5f,EC2);
    chsr(400, 380, 540, 1.9f,EC2); chsr(1240,1220,1380, 1.9f,EC2);
    mush(1080); chkpt(900);
}

static void s2l3() {
    // Shooter's Gallery — narrower platforms, 2 teal shooters
    g.levelWidth = 2600.f;
    gnd(0,260); gnd(380,160); gnd(620,160); gnd(880,180);
    gnd(1120,180); gnd(1360,200); gnd(1600,180); gnd(2300,300);
    flt(400,315,110); flt(640,320,100);
    flt(900,295,110); flt(1140,305,100);
    flt(1380,310,110); flt(1620,300,110);
    spk(600,28); spk(858,28); spk(1340,24); spk(1580,28);
    cns(410,293); cns(650,298);
    cns(910,273, 2); cns(1150,283, 2);
    cns(1390,288, 2); cns(1630,278);
    chsr(420, 380, 520, 2.0f,EC2); chsr(660, 620, 760, 2.0f,EC2);
    chsr(900, 880,1060, 2.0f,EC2);
    shtr(1140,1120,1300, 2.2f,EC2);
    chsr(1390,1360,1540, 2.0f,EC2);
    shtr(1640,1600,1760, 2.0f,EC2);
    flyr(2050,280,1980,2260, 255,310, 1.6f,EC2);
    mush(1250); chkpt(850);
}

static void s2l4() {
    // Crystal Gauntlet — chain-kill x5, spikes everywhere
    g.levelWidth = 2800.f;
    gnd(0,260); gnd(380,160); gnd(640,160); gnd(900,200);
    gnd(1160,200); gnd(1420,200); gnd(1680,200); gnd(2500,300);
    flt(400,310,100); flt(660,320,100);
    flt(920,300,120); flt(1180,310,110);
    flt(1700,300,120); flt(1960,295,110);
    spk(620,24); spk(878,24); spk(1140,28); spk(1658,24); spk(1940,24);
    cns(920, GY-22.f, 4); cns(930, 278, 2);
    cns(1180,GY-22.f, 2); cns(1190,288, 2);
    cns(1710,278); cns(1970,278);
    chsr(940, 900,1060, 2.1f,EC2); chsr(1040, 900,1160, 2.1f,EC2);
    chsr(1160,1000,1320, 2.1f,EC2); shtr(1260,1160,1380, 2.0f,EC2);
    jmpr(1440,1420,1600, 1.8f,EC2);
    chsr(420, 380, 520, 2.0f,EC2); chsr(680, 640, 780, 2.0f,EC2);
    shtr(1720,1680,1840, 2.2f,EC2);
    flyr(1980,280,1920,2200, 255,310, 1.7f,EC2);
    chsr(2200,2140,2460, 2.1f,EC2);
    mush(1350); mush(2080); chkpt(860);
}

static void s2l5() {
    // Stalactite Maze — teal flyers and spikes forcing vertical awareness
    g.levelWidth = 2600.f;
    gnd(0,260); gnd(380,160); gnd(640,180); gnd(900,180);
    gnd(1160,180); gnd(1420,200); gnd(1680,200); gnd(2300,300);
    flt(400,310,110); flt(660,295,100);
    flt(920,280,110); flt(1180,295,100);
    flt(1440,305,110); flt(1700,285,120);
    spk(620,28); spk(878,28); spk(1140,24); spk(1400,28); spk(1660,28);
    cns(410,288,2); cns(670,273,2); cns(930,258,2);
    cns(1190,273,2); cns(1450,283,2); cns(1720,263,2);
    chsr(420, 380, 520, 2.1f,EC2); chsr(680, 640, 760, 2.1f,EC2);
    flyr(760, 265, 640, 880, 240,295, 1.6f,EC2);
    chsr(940, 900,1060, 2.1f,EC2);
    flyr(1050,270,900,1140, 245,290, 1.7f,EC2);
    shtr(1180,1160,1340, 2.1f,EC2);
    flyr(1300,268,1160,1420, 243,288, 1.7f,EC2);
    chsr(1460,1420,1580, 2.1f,EC2);
    shtr(1720,1680,1840, 2.0f,EC2);
    flyr(2000,270,1940,2200, 245,295, 1.8f,EC2);
    mush(1060); mush(1900); chkpt(860);
}

static void s2l6() {
    // WOW — Vanishing Vault: most platforms vanish; time your dashes or fall
    g.levelWidth = 2800.f;
    gnd(0,260); gnd(420,140); gnd(680,140); gnd(960,160);
    gnd(1240,160); gnd(1520,160); gnd(1800,160); gnd(2500,300);
    // Vanishing floaters — the gimmick
    fltv(440,315,110); fltv(700,305,100); fltv(980,295,110);
    fltv(1260,305,100); fltv(1540,300,110);
    flt(1820,290,110); // one safe platform near end as relief
    fltv(2100,295,100); fltv(2300,285,110);
    spk(660,28); spk(920,24); spk(1200,28); spk(1460,24); spk(1760,28);
    cns(445,293,2); cns(705,283,2); cns(985,273,2);
    cns(1265,283,2); cns(1545,278,2); cns(2110,273,2);
    chsr(460, 420, 540, 2.2f,EC2); chsr(720, 680, 800, 2.2f,EC2);
    chsr(1000, 960,1100, 2.2f,EC2); shtr(1260,1240,1380, 2.0f,EC2);
    jmpr(1560,1520,1660, 2.0f,EC2);
    flyr(1840,275,1780,2060, 250,305, 1.9f,EC2);
    chsr(2120,2080,2280, 2.2f,EC2); shtr(2320,2280,2460, 2.0f,EC2);
    mush(1100); mush(2160); chkpt(900); chkpt(1480);
}

static void s2l7() {
    // Crystal Crossfire — 6 shooters at varied heights, dodge-fest
    g.levelWidth = 3000.f;
    gnd(0,260); gnd(380,160); gnd(640,160); gnd(900,180);
    gnd(1160,180); gnd(1440,200); gnd(1720,180); gnd(2000,200); gnd(2600,300);
    flt(400,310,110); flt(660,290,100);
    flt(920,275,110); flt(1180,295,100);
    flt(1460,280,110); flt(1740,265,110);
    flt(2020,275,100); flt(2200,255,110);
    spk(620,24); spk(878,28); spk(1140,24); spk(1400,28); spk(1700,24); spk(1980,28);
    cns(410,288,2); cns(670,268,2); cns(930,253,2);
    cns(1190,273,2); cns(1470,258,2); cns(2030,253,2);
    // Shooters on multiple height levels — cross-fire
    shtr( 420, 380, 520, 2.0f,EC2); shtr( 680, 640, 760, 1.8f,EC2);
    chsr( 940, 900,1060, 2.2f,EC2);
    shtr(1200,1160,1340, 2.0f,EC2);
    flyr(1100,268, 900,1160, 243,293, 1.8f,EC2);
    shtr(1480,1440,1620, 1.8f,EC2);
    chsr(1760,1720,1860, 2.2f,EC2);
    shtr(2040,2000,2180, 1.8f,EC2); shtr(2220,2160,2360, 1.8f,EC2);
    flyr(2320,248,2200,2560, 223,278, 2.0f,EC2);
    mush(1060); mush(2100); chkpt(900); chkpt(1600);
}

static void s2l8() {
    // BOSS: Sentinel — health 3, crystal spikes + geyser in arena
    g.levelWidth = 2000.f;
    gnd(0,300); gnd(400,180); gnd(660,180);
    gnd(920,1080);
    flt(1000,340,150); flt(1520,340,150); // elevated cover platforms
    flt(400,310,110); flt(670,315,110);
    spk(620,24); spk(878,24);
    // Arena hazards: 3 spike clusters + geyser
    spk(1060,24); spk(1380,24); spk(1700,28);
    geysr(1240, 2.8f);
    cns(415,288,2); cns(685,293,2);
    cns(1010,318,2); cns(1530,318,2);
    chkpt(760); mush(700); mush(1240);
    bss(1380, 940,1800, 3, 1.8f, EC2);
}

// ── Stage 3: Volcano Peak  (theme 4 · bgm[4]) ────────────────────────────────

static void s3l1() {
    // Cinder Path — geysers in 2 gaps, fast orange chasers intro
    g.levelWidth = 2600.f;
    gnd(0,260); gnd(380,180); gnd(660,180); gnd(940,200);
    gnd(1220,200); gnd(1500,200); gnd(1780,200); gnd(2300,300);
    flt(400,315,120); flt(960,305,110); flt(1520,310,120);
    spk(620,24); spk(900,28); spk(1460,24); spk(1760,28);
    geysr(320, 2.8f); geysr(850, 3.2f);
    cns(410,293); cns(980, GY-22.f, 2);
    cns(990,283, 2); cns(1540,288);
    cns(1800,GY-22.f, 2);
    chsr(420, 380, 540, 2.2f,EC3); chsr(700, 660, 820, 2.2f,EC3);
    chsr(980, 940,1100, 2.2f,EC3); chsr(1560,1500,1660, 2.2f,EC3);
    flyr(1820,285,1760,2000, 260,310, 1.7f,EC3);
    mush(1100); chkpt(880);
}

static void s3l2() {
    // Lava Spire — 3 geysers, shooters appear
    g.levelWidth = 2600.f;
    gnd(0,260); gnd(380,160); gnd(640,160); gnd(900,180);
    gnd(1160,180); gnd(1420,200); gnd(1680,180); gnd(2300,300);
    flt(400,315,110); flt(660,320,100);
    flt(920,300,110); flt(1440,305,110); flt(1700,310,110);
    spk(620,28); spk(878,28); spk(1140,28); spk(1660,24);
    geysr(320, 2.5f); geysr(800, 2.8f); geysr(1300, 3.0f);
    cns(410,293); cns(670,298, 2);
    cns(940,278, 2); cns(1450,283);
    cns(1720,288, 2);
    chsr(420, 380, 520, 2.3f,EC3); chsr(680, 640, 760, 2.3f,EC3);
    chsr(940, 900,1060, 2.3f,EC3);
    shtr(1180,1160,1360, 2.0f,EC3);
    chsr(1460,1420,1580, 2.3f,EC3);
    shtr(1720,1680,1840, 2.0f,EC3);
    flyr(2000,280,1940,2240, 255,310, 1.8f,EC3);
    mush(1280); chkpt(860);
}

static void s3l3() {
    // Magma Climb — vertical two-step platforms, spikes + geyser combos
    g.levelWidth = 2800.f;
    gnd(0,260); gnd(380,160); gnd(640,160); gnd(900,180);
    gnd(1160,180); gnd(1440,200); gnd(1700,180); gnd(2500,300);
    flt(400,315,110); flt(660,325,100);
    flt(920,285,100); flt(1000,245,90);
    flt(1180,295,110);
    flt(1460,305,110, 1.5f,0, 1400,1660);
    flt(1720,285,110);
    spk(620,28); spk(878,28); spk(1140,24); spk(1420,28); spk(1680,24);
    geysr(320, 2.5f); geysr(820, 2.6f); geysr(1080, 2.8f); geysr(1380, 3.0f);
    cns(410,293); cns(670,303, 2);
    cns(930,263, 2); cns(1010,223, 2);
    cns(1190,273, 2); cns(1470,283, 2);
    chsr(420, 380, 520, 2.4f,EC3); chsr(680, 640, 760, 2.4f,EC3);
    chsr(940, 900,1060, 2.4f,EC3);
    shtr(1180,1160,1340, 2.0f,EC3); shtr(1380,1300,1600, 1.8f,EC3);
    jmpr(1480,1440,1620, 2.0f,EC3);
    chsr(1740,1700,1860, 2.4f,EC3);
    flyr(2100,275,2040,2320, 250,305, 1.9f,EC3);
    mush(1300); chkpt(860);
}

static void s3l4() {
    // Inferno Rush — maximum density, 4 geysers, chain-kill x5
    g.levelWidth = 2800.f;
    gnd(0,260); gnd(380,160); gnd(640,160); gnd(900,180);
    gnd(1160,180); gnd(1420,200); gnd(1680,180); gnd(2500,300);
    flt(400,310,100); flt(660,315,100);
    flt(920,295,120); flt(1180,305,110);
    flt(1700,295,120); flt(1960,300,110);
    spk(620,28); spk(878,28); spk(1140,24); spk(1400,28); spk(1660,24); spk(1940,24);
    geysr(320, 2.2f); geysr(800, 2.4f); geysr(1300, 2.5f); geysr(1600, 2.8f);
    cns(920, GY-22.f, 4); cns(1160,GY-22.f, 4);
    cns(930, 273, 2); cns(1190,283, 2);
    cns(1710,273); cns(1970,278);
    chsr(940, 900,1060, 2.5f,EC3); chsr(1040, 900,1160, 2.5f,EC3);
    chsr(1180,1060,1320, 2.5f,EC3); shtr(1280,1160,1400, 1.8f,EC3);
    jmpr(1440,1420,1600, 2.2f,EC3);
    chsr(420, 380, 520, 2.3f,EC3); chsr(680, 640, 760, 2.3f,EC3);
    shtr(1720,1680,1840, 2.0f,EC3);
    flyr(1980,278,1920,2200, 253,308, 2.0f,EC3);
    chsr(2200,2140,2460, 2.5f,EC3);
    shtr(2400,2380,2480, 2.0f,EC3);
    mush(1350); mush(2100); chkpt(860);
}

static void s3l5() {
    // WOW — Eruption: lava rises from the start, race the whole level
    g.levelWidth = 2800.f;
    g.risingLavaY = 520.f;  // lava already rising — beat the clock
    gnd(0,220); gnd(320,160); gnd(580,160); gnd(840,180);
    gnd(1100,160); gnd(1360,160); gnd(1620,160); gnd(2500,300);
    flt(340,305,110); flt(600,295,100); flt(860,280,110);
    flt(1120,270,100); flt(1380,265,110); flt(1640,275,110);
    flt(1900,260,120); flt(2100,250,100);
    spk(560,28); spk(800,28); spk(1060,24); spk(1320,28); spk(1600,24);
    geysr(240, 2.0f); geysr(520, 2.2f); geysr(760, 2.4f); geysr(1020, 2.6f);
    cns(350,283,2); cns(610,273,2); cns(870,258,2);
    cns(1130,248,2); cns(1390,243,2); cns(1910,238,2);
    chsr(360, 320, 440, 2.6f,EC3); chsr(620, 580, 720, 2.6f,EC3);
    chsr(880, 840,1000, 2.6f,EC3);
    shtr(1120,1100,1260, 1.8f,EC3);
    chsr(1400,1360,1500, 2.6f,EC3);
    shtr(1660,1620,1760, 1.8f,EC3);
    flyr(1920,248,1860,2100, 223,273, 2.1f,EC3);
    chsr(2120,2060,2300, 2.6f,EC3);
    mush(920); mush(1760); chkpt(800); chkpt(1440);
}

static void s3l6() {
    // Magma Surge — WOW lava chase: lava starts on screen, dash or die
    g.levelWidth = 3200.f;
    g.risingLavaY = 600.f;  // lava starts just below screen
    gnd(0,200); gnd(300,140); gnd(540,140); gnd(780,160);
    gnd(1020,140); gnd(1260,160); gnd(1500,140); gnd(1740,160);
    gnd(2000,140); gnd(2260,160); gnd(2900,300);
    flt(320,295,100); flt(560,285,100); flt(800,275,110);
    flt(1040,265,100); flt(1280,270,110); flt(1520,260,100);
    flt(1760,268,110); flt(2020,258,100); flt(2280,263,110);
    spk(480,28); spk(720,24); spk(960,28); spk(1200,24); spk(1440,28);
    spk(1700,24); spk(1960,28); spk(2240,24);
    geysr(240,1.8f); geysr(480,2.0f); geysr(720,1.9f); geysr(960,2.1f);
    geysr(1200,2.0f); geysr(1440,1.8f);
    cns(330,273,2); cns(570,263,2); cns(810,253,2);
    cns(1050,243,2); cns(1290,248,2); cns(1770,246,2);
    chsr(340, 300, 420, 2.8f,EC3); chsr(580, 540, 660, 2.8f,EC3);
    chsr(820, 780, 920, 2.8f,EC3); chsr(1060,1020,1160, 2.8f,EC3);
    shtr(1300,1260,1420, 1.6f,EC3); chsr(1540,1500,1640, 2.8f,EC3);
    shtr(1780,1740,1880, 1.6f,EC3); chsr(2040,2000,2140, 2.8f,EC3);
    jmpr(2300,2260,2420, 2.4f,EC3);
    flyr(2440,252,2360,2640, 227,277, 2.2f,EC3);
    mush(840); mush(1560); mush(2460); chkpt(780); chkpt(1500); chkpt(2260);
}

static void s3l7() {
    // Final Approach — 6 geysers, max density, pre-boss showdown
    g.levelWidth = 3000.f;
    gnd(0,260); gnd(380,160); gnd(640,160); gnd(900,180);
    gnd(1160,180); gnd(1440,200); gnd(1720,180); gnd(2000,200);
    gnd(2280,180); gnd(2700,300);
    flt(400,310,100); flt(660,300,100);
    flt(920,285,110); flt(1180,295,100);
    flt(1460,278,110); flt(1740,268,110);
    flt(2020,273,100); flt(2300,258,110);
    spk(620,28); spk(878,28); spk(1140,24); spk(1400,28);
    spk(1680,24); spk(1960,28); spk(2240,24);
    geysr(320,2.0f); geysr(780,2.2f); geysr(1060,2.3f);
    geysr(1380,2.0f); geysr(1660,2.2f); geysr(1980,2.4f);
    cns(410,288,3); cns(670,278,2); cns(930,263,2);
    cns(1190,273,2); cns(1470,256,2); cns(2030,251,2);
    chsr(420, 380, 520, 2.6f,EC3); chsr(680, 640, 760, 2.6f,EC3);
    chsr(940, 900,1060, 2.6f,EC3); shtr(1180,1160,1340, 1.8f,EC3);
    jmpr(1460,1440,1600, 2.3f,EC3);
    chsr(1760,1720,1880, 2.6f,EC3); shtr(2020,2000,2180, 1.8f,EC3);
    chsr(2320,2280,2440, 2.6f,EC3); shtr(2460,2400,2600, 1.8f,EC3);
    flyr(2560,251,2460,2680, 226,276, 2.2f,EC3);
    mush(1060); mush(2100); chkpt(880); chkpt(1600);
}

static void s3l8() {
    // BOSS: Inferno — health 4, arena with moving platform + geyser + spikes
    g.levelWidth = 2000.f;
    gnd(0,300); gnd(400,180); gnd(660,180);
    gnd(920,1080);
    flt(960, 335,140); flt(1520,335,140); // low platforms
    flt(1200,270,120);                    // high central perch
    flt(1100,310,100, 1.6f,0, 980,1560);  // moving platform — arena hazard
    flt(400,310,110); flt(670,315,110);
    spk(620,28); spk(878,28);
    // Arena hazards: spikes + geyser
    spk(1040,24); spk(1640,24);
    geysr(1360, 2.2f);
    geysr(350, 2.5f); geysr(620, 2.8f);
    cns(415,288,2); cns(685,293,2);
    cns(970,313,2); cns(1210,248,2);
    chkpt(760); mush(700); mush(960);
    bss(1380, 940,1800, 4, 2.0f, EC3);
}

// ── Stage 1: new levels (s1l5 through s1l8) ──────────────────────────────────

static void s1l5() {
    // Split Decision — two viable routes: safe high road vs risky low road
    g.levelWidth = 2600.f;
    gnd(0,260); gnd(380,180); gnd(640,200);
    gnd(900,180); gnd(1160,180); gnd(1420,200); gnd(1680,200); gnd(2300,300);
    // High road floaters (safe, fewer enemies, less loot)
    flt(400,295,110); flt(660,285,110);
    flt(920,270,110); flt(1180,285,110);
    flt(1440,275,120); flt(1700,285,110);
    // Low road has spikes — faster, more coins, chain clusters
    spk(628,28); spk(878,28); spk(1140,24); spk(1658,28);
    cns(410,273,2); cns(670,263,2); cns(930,248,3);
    cns(1190,263,2); cns(1450,253,2);
    cns(920, GY-22.f,3); cns(1160,GY-22.f,3);
    // Ground enemies in chain cluster
    chsr(940, 900,1060, 2.0f); chsr(1040, 900,1180, 2.0f);
    chsr(1180,1060,1320, 2.0f);
    chsr(420, 380, 540, 1.9f); chsr(680, 640, 780, 1.9f);
    jmpr(1460,1420,1580, 1.7f);
    flyr(1720,272,1680,1900, 247,292, 1.6f);
    chsr(1900,1860,2020, 2.0f); shtr(2100,2060,2240, 2.1f);
    mush(1100); mush(2000); chkpt(880);
}

static void s1l6() {
    // Speedrun Alley — WOW level: ultra-long corridor, dash everything
    g.levelWidth = 3400.f;
    gnd(0,220); gnd(320,180); gnd(580,180); gnd(840,200);
    gnd(1100,200); gnd(1360,180); gnd(1620,200); gnd(1880,200);
    gnd(2140,180); gnd(2400,200); gnd(2660,200); gnd(3100,300);
    flt(340,305,100); flt(600,300,100); flt(860,290,110);
    flt(1120,295,100); flt(1380,285,110); flt(1640,290,100);
    flt(1900,285,110); flt(2160,275,110); flt(2420,280,100);
    // Chain clusters every 500px — designed for non-stop dashing
    chsr(360, 320, 440, 2.2f); chsr(440, 320, 520, 2.2f);
    chsr(620, 580, 700, 2.2f); chsr(700, 580, 780, 2.2f);
    chsr(880, 840, 960, 2.2f); chsr(960, 840,1040, 2.2f);
    chsr(1140,1100,1220, 2.2f); chsr(1220,1100,1300, 2.2f);
    jmpr(1400,1360,1480, 1.8f);
    chsr(1660,1620,1740, 2.4f); chsr(1740,1620,1820, 2.4f);
    shtr(1920,1880,2040, 2.2f);
    chsr(2180,2140,2260, 2.4f); chsr(2260,2140,2340, 2.4f);
    shtr(2440,2400,2560, 2.2f); shtr(2600,2560,2720, 2.2f);
    flyr(2720,275,2660,2920, 250,305, 2.0f);
    cns(360,GY-22.f,4,22.f); cns(880,GY-22.f,4,22.f);
    cns(1400,GY-22.f,4,22.f); cns(1900,GY-22.f,4,22.f);
    cns(2400,GY-22.f,4,22.f);
    cns(345,283,2); cns(605,278,2); cns(865,268,2); cns(1385,263,2);
    mush(800); mush(1600); mush(2500); chkpt(900); chkpt(1800);
}

static void s1l7() {
    // Chain Garden — mastery test: 4 chain clusters, jumpers, moving platforms
    g.levelWidth = 2800.f;
    gnd(0,260); gnd(380,180); gnd(640,180); gnd(900,200);
    gnd(1160,200); gnd(1440,200); gnd(1700,200); gnd(2500,300);
    flt(400,305,110); flt(660,310,100);
    flt(700,290,100, 1.4f,0, 640,880);   // moving — chain while dodging
    flt(960,280,110); flt(1220,295,100);
    flt(1460,290,110, 1.5f,0, 1400,1660);
    flt(1720,280,110); flt(1960,270,100);
    cns(920,GY-22.f,4); cns(1160,GY-22.f,4); // chain cluster rewards
    cns(1700,GY-22.f,4); cns(410,283,2); cns(670,288,2);
    cns(970,258,2); cns(1230,273,2); cns(1730,258,2); cns(1970,248,2);
    // Four distinct chain clusters — 3 enemies each, spaced for dash chains
    chsr(940, 900,1040, 2.1f); chsr(1020, 900,1160, 2.1f); chsr(1160,1000,1280, 2.1f);
    jmpr(1000, 960,1100, 1.9f);  // jumper in cluster = aerial dash needed
    chsr(1480,1440,1580, 2.2f); chsr(1560,1440,1660, 2.2f);
    jmpr(1500,1460,1620, 1.9f);
    chsr(1740,1700,1840, 2.2f); chsr(1820,1700,1940, 2.2f);
    shtr(1960,1920,2100, 2.1f);
    chsr(420, 380, 520, 2.0f); chsr(680, 640, 760, 2.0f);
    flyr(2120,268,2060,2320, 243,293, 2.0f);
    chsr(2340,2300,2460, 2.2f);
    mush(1080); mush(2160); chkpt(900); chkpt(1540);
}

static void s1l8() {
    // BOSS: Warden — health 2, arena with 2 spike hazards + moving platform
    g.levelWidth = 2000.f;
    gnd(0,300); gnd(390,200); gnd(670,200);
    gnd(940,1060);
    flt(1080,330,130); flt(1580,330,130); // cover platforms
    flt(1280,310,110, 1.4f,0, 1020,1760); // moving platform — dodge shots while riding
    flt(400,310,120); flt(680,320,120);
    // Arena hazards: 2 spike clusters
    spk(1050,28); spk(1700,28);
    cns(415,288,2); cns(695,298,2);
    cns(1090,308,2); cns(1590,308,2);
    chkpt(760); mush(700);
    bss(1400, 960,1800, 2, 1.4f);
}

// ── Level dispatch ────────────────────────────────────────────────────────────
using LevelFn = void(*)();
static const LevelFn LEVELS[4][9] = {
    {},
    {nullptr, s1l1, s1l2, s1l3, s1l4, s1l5, s1l6, s1l7, s1l8},
    {nullptr, s2l1, s2l2, s2l3, s2l4, s2l5, s2l6, s2l7, s2l8},
    {nullptr, s3l1, s3l2, s3l3, s3l4, s3l5, s3l6, s3l7, s3l8},
};

void initLevel() {
    g.platforms.clear();
    g.coins.clear();
    g.enemies.clear();
    g.mushrooms.clear();
    g.checkpoints.clear();
    g.spikes.clear();
    g.geysers.clear();

    if (g.currentStage >= 1 && g.currentStage <= 3 &&
        g.levelInStage >= 1 && g.levelInStage <= 8) {
        LEVELS[g.currentStage][g.levelInStage]();
    } else {
        float lw = 2800.f;
        generateLevel(g.currentStage, g.levelInStage,
                      g.platforms, g.coins, g.enemies,
                      g.mushrooms, g.checkpoints, g.spikes, g.geysers, lw);
        g.levelWidth = lw;
    }

    g.levelCompleteTimer = 0.f;
    g.currentLevel = (g.currentStage == 2) ? 3 : (g.currentStage == 3) ? 4 : g.currentStage;

    g.projectiles.clear();
    g.particles.clear();
    g.popups.clear();
    g.cameraX  = 0.f;
    g.player   = Player{};
    g.maxCombo = 0;

    g.windForce   = 0.f;
    g.risingLavaY = 9999.f;

    g.levelDeaths    = 0;
    g.levelStartTime = g.elapsed;
    g.timeScale = 1.f; g.timeScaleTimer = 0.f; g.vignettePulse = 0.f;
    g.cameraLookahead = 0.f;
}
