#include "render.h"
#include "globals.h"
#include "game.h"
#include "audio.h"
#include "procgen.h"
#include "Tuning.h"
#include <cmath>
#include <string>
#include <algorithm>
#include <cstdio>
#include <fstream>

// ── Procedural tile textures ──────────────────────────────────────────────────
static Texture2D tileTex[7]; // 0=grass,1=forest,2=stone,3=moving,4=volcanic,5=ice,6=metal
static const int TILE = 32;

static Color noiseMix(Color a, Color b, int x, int y, int seed) {
    return ((x*17 + y*31 + seed) % 7) < 2 ? b : a;
}

void initSprites() {
    for (int t = 0; t < 7; t++) {
        Image img = GenImageColor(TILE, TILE, BLACK);
        for (int y = 0; y < TILE; y++) for (int x = 0; x < TILE; x++) {
            Color c;
            if (t == 0) {
                if (y < 5)       c = noiseMix({92,200,80,255},  {70,185,55,255},  x, y, 3);
                else if (y < 10) c = noiseMix({72,155,60,255},  {58,130,45,255},  x, y, 7);
                else             c = noiseMix({100,68,32,255},   {80,52,24,255},   x, y, 11);
                if (y < 3 && (x%4 < 2)) c = {60,220,50,255};
                if (y > 10 && (y%8 == 0 || y%8 == 1)) c = {60,40,18,255};
                if (y > 10 && (x%16 == 0))             c = {60,40,18,255};
            } else if (t == 1) {
                if (y < 5)       c = noiseMix({55,130,65,255},  {40,105,48,255},  x, y, 5);
                else if (y < 10) c = noiseMix({38,92,48,255},   {28,72,36,255},   x, y, 9);
                else             c = noiseMix({72,52,28,255},    {55,38,18,255},   x, y, 13);
                if (y < 3 && (x%4 < 2)) c = {45,180,55,255};
                if (y > 10 && (y%8 == 0 || y%8 == 1)) c = {40,28,12,255};
                if (y > 10 && (x%16 == 0))             c = {40,28,12,255};
            } else if (t == 2) {
                if (y < 6) c = noiseMix({108,55,165,255}, {88,42,140,255}, x, y, 17);
                else       c = noiseMix({60,24,95,255},    {45,16,72,255},  x, y, 23);
                int rx = x%16, ry = y%16;
                if ((rx==7||rx==8) && ry>3 && ry<12) c = {140,80,210,180};
                if ((ry==7||ry==8) && rx>3 && rx<12) c = {140,80,210,180};
                if (y < 6 && (y%6==0||(y%6==1))) c = {30,10,50,255};
                if (y < 6 && (x%16==0))          c = {30,10,50,255};
            } else if (t == 3) {
                if (y < 6) c = noiseMix({140,210,255,255}, {115,190,240,255}, x, y, 19);
                else       c = noiseMix({60,140,220,255},   {45,115,200,255},  x, y, 29);
                if ((x+y)%10==0)    c = {200,235,255,180};
                if (y < 6 && (y%6==0)) c = {30,80,160,255};
            } else if (t == 4) {
                if (y < 6) c = noiseMix({60,20,10,255}, {45,15,8,255}, x, y, 37);
                else       c = noiseMix({40,14,6,255},   {30,10,4,255}, x, y, 41);
                int rx = x%16, ry = y%14;
                if ((rx==7||rx==8) && ry>2 && ry<11) c = {200,70,10,180};
                if ((ry==5||ry==6) && rx>2 && rx<14) c = {200,70,10,160};
                if (y < 6 && (y%6==0||(y%6==1))) c = {20,8,4,255};
                if (y < 6 && (x%16==0))          c = {20,8,4,255};
            } else if (t == 5) {
                if (y < 6) c = noiseMix({190,220,250,255}, {170,205,240,255}, x, y, 43);
                else       c = noiseMix({130,175,220,255},  {110,155,205,255}, x, y, 47);
                if ((x+y)%8==0)       c = {220,240,255,200};
                if ((x*3+y*2)%12==0)  c = {240,250,255,160};
                if (y < 6 && (y%6==0)) c = {80,130,190,255};
            } else {
                if (y < 6) c = noiseMix({55,60,70,255}, {45,50,58,255}, x, y, 53);
                else       c = noiseMix({35,38,45,255}, {28,30,36,255}, x, y, 59);
                if (x%16==8 && y%16==8) c = {80,90,100,255};
                if ((x%16==0||x%16==1) && y>6) c = {22,24,30,255};
                if (y < 6 && (y%6==0))  c = {20,22,28,255};
                if ((x+y)%20==0)         c = {65,70,80,180};
            }
            ImageDrawPixel(&img, x, y, c);
        }
        tileTex[t] = LoadTextureFromImage(img);
        SetTextureFilter(tileTex[t], TEXTURE_FILTER_POINT);
        UnloadImage(img);
    }
}

void deinitSprites() {
    for (int i = 0; i < 7; i++) UnloadTexture(tileTex[i]);
}

void fillRect(float x, float y, float w, float h, Color c) {
    DrawRectangle((int)x, (int)y, (int)w, (int)h, c);
}

static void drawBloom(float cx, float cy, float r, Color c, int layers = 4) {
    float step = r * 0.30f;
    for (int i = layers; i > 0; i--) {
        Color gc = c; gc.a = (unsigned char)(c.a * i / (layers * 2));
        DrawCircle((int)cx, (int)cy, (int)(r + i * step), gc);
    }
}

// ── Neon cyberpunk helpers ─────────────────────────────────────────────────────

static void drawScanlines(unsigned char alpha = 18) {
    for (int ys = 0; ys < SH; ys += 3)
        DrawLine(0, ys, SW, ys, {0,0,0,alpha});
}

static void drawCyberGrid(float t, unsigned char alpha = 22) {
    float ox = fmodf(t * 14.f, 48.f), oy = fmodf(t * 9.f, 48.f);
    Color gc = {0,255,200,alpha};
    for (float gx = -ox; gx < SW+48; gx += 48) DrawLineEx({gx,0},{gx,(float)SH},0.5f,gc);
    for (float gy = -oy; gy < SH+48; gy += 48) DrawLineEx({0,gy},{(float)SW,gy},0.5f,gc);
}

static void drawNeonText(const char* txt, int x, int y, int fs, Color c) {
    Color g2 = c; g2.a = (unsigned char)(c.a * 0.10f);
    Color g1 = c; g1.a = (unsigned char)(c.a * 0.22f);
    DrawText(txt,x-2,y,fs,g2); DrawText(txt,x+2,y,fs,g2);
    DrawText(txt,x,y-2,fs,g2); DrawText(txt,x,y+2,fs,g2);
    DrawText(txt,x-1,y,fs,g1); DrawText(txt,x+1,y,fs,g1);
    DrawText(txt,x,y-1,fs,g1); DrawText(txt,x,y+1,fs,g1);
    DrawText(txt,x,y,fs,c);
}

static void drawCornerBrackets(float x, float y, float w, float h, float sz, Color c) {
    DrawLineEx({x,y+sz},{x,y},2.f,c);       DrawLineEx({x,y},{x+sz,y},2.f,c);
    DrawLineEx({x+w-sz,y},{x+w,y},2.f,c);   DrawLineEx({x+w,y},{x+w,y+sz},2.f,c);
    DrawLineEx({x,y+h-sz},{x,y+h},2.f,c);   DrawLineEx({x,y+h},{x+sz,y+h},2.f,c);
    DrawLineEx({x+w-sz,y+h},{x+w,y+h},2.f,c); DrawLineEx({x+w,y+h-sz},{x+w,y+h},2.f,c);
}

// ── UI helpers ────────────────────────────────────────────────────────────────
bool drawBtn(float x, float y, float w, float h, const char* txt, Color bg, Color hov, int fs) {
    Vector2 mp = g.mouse;
    bool over    = CheckCollisionPointRec(mp, {x, y, w, h});
    bool clicked = over && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    float glow   = 0.5f + 0.5f * sinf(g.menuTimer * 5.f);
    if (over) {
        for (int i = 5; i > 0; i--) {
            unsigned char ga = (unsigned char)(glow * 26.f / i);
            DrawRectangleLinesEx({x-i, y-i, w+i*2, h+i*2}, 1.f, {0,255,200,ga});
        }
    }
    DrawRectangle((int)x, (int)y, (int)w, (int)h, {5,8,20,235});
    DrawRectangle((int)x, (int)y, (int)w, 1, over ? Color{0,255,200,180} : Color{0,255,200,45});
    Color bc = over ? Color{0,255,200,230} : Color{0,255,200,90};
    DrawRectangleLinesEx({x, y, w, h}, 1.5f, bc);
    float cs = std::min(8.f, h * 0.28f);
    drawCornerBrackets(x, y, w, h, cs, over ? Color{0,255,200,255} : Color{0,255,200,130});
    if (over) {
        float sweep = fmodf(g.menuTimer * 88.f, h + 14.f) - 7.f;
        if (sweep >= 0 && sweep < h)
            DrawRectangle((int)x+1, (int)(y+sweep), (int)w-2, 2, {0,255,200,28});
    }
    int tw = MeasureText(txt, fs), tx2 = (int)(x+w/2-tw/2), ty2 = (int)(y+h/2-fs/2);
    if (over) drawNeonText(txt, tx2, ty2, fs, {0,255,200,255});
    else      DrawText(txt, tx2, ty2, fs, {210,240,255,230});
    return clicked;
}

void drawCloseBtn() {
    Vector2 mp = g.mouse;
    bool over = CheckCollisionPointRec(mp, {(float)SW-32, 4, 28, 28});
    float glow = 0.5f + 0.5f * sinf(g.menuTimer * 4.f);
    if (over) {
        for (int i = 3; i > 0; i--)
            DrawRectangleLinesEx({(float)(SW-32-i),(float)(4-i),28+i*2.f,28+i*2.f},1.f,
                {255,50,80,(unsigned char)(20/i)});
    }
    DrawRectangle(SW-32, 4, 28, 28, {5,8,20,230});
    Color xc = over ? Color{255,80,100,255} : Color{0,255,200,160};
    DrawRectangleLinesEx({(float)(SW-32),4,28,28}, 1.5f, xc);
    drawCornerBrackets(SW-32.f,4,28,28,5.f, over ? Color{255,80,100,200} : Color{0,255,200,90});
    int xw = MeasureText("X", 16);
    if (over) drawNeonText("X", SW-32+14-xw/2, 10, 16, {255,80,100,255});
    else      DrawText("X", SW-32+14-xw/2, 10, 16, {0,255,200,200});
    if (over && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) CloseWindow();
}

// ── Player ────────────────────────────────────────────────────────────────────
void drawPlayerAt(float px, float py, int facing, int walkFrame, bool grounded, bool powered, int invFrames) {
    if (invFrames > 0 && (invFrames/5)%2 == 0) return;

    const float w = 28.f, h = 36.f;
    Color bodyC = {0, 245, 255, 255};
    Color glowC = powered ? Color{255, 190, 11, 255} : bodyC;

    // Bloom glow layers (outermost first so body renders on top)
    for (int i = 4; i > 0; i--) {
        float pad = i * 6.f;
        unsigned char ga = (unsigned char)(powered ? 20 * i : 13 * i);
        DrawRectangleRounded(
            {px - pad, py - pad, w + pad * 2.f, h + pad * 2.f},
            0.35f, 5, {glowC.r, glowC.g, glowC.b, ga});
    }

    // Body
    DrawRectangleRounded({px, py, w, h}, 0.28f, 5, bodyC);

    // Surface sheen
    DrawRectangleRounded({px + 3.f, py + 2.f, 20.f, 8.f}, 0.6f, 4, {200, 255, 255, 55});

    // Directional bump on leading side
    float bX = facing > 0 ? px + w : px - 5.f;
    DrawRectangleRounded({bX, py + h * 0.35f, 5.f, h * 0.3f}, 0.6f, 4, bodyC);

    // Eye dot — direction indicator
    float eX = facing > 0 ? px + w - 7.f : px + 5.f;
    DrawCircle((int)eX, (int)(py + 10.f), 3, WHITE);
    DrawCircle((int)(eX + facing), (int)(py + 11.f), 2, {0, 50, 80, 255});

    // Powered outline ring
    if (powered) {
        float glw = 0.55f + 0.45f * sinf(g.menuTimer * 5.f);
        DrawRectangleLinesEx({px - 3.f, py - 3.f, w + 6.f, h + 6.f}, 2.f,
            {255, 190, 11, (unsigned char)(glw * 220)});
    }
}

// ── Enemy death animation ──────────────────────────────────────────────────────
void drawDeathAnim(const Enemy& e) {
    float p = std::max(0.f, e.deathTimer) / 0.35f;
    float ew = (e.type==3) ? 48.f : 28.f, eh = (e.type==3) ? 60.f : 28.f;
    float sc = p*p; float nw = ew*sc, nh = eh*sc;
    if (nw < 1 || nh < 1) return;
    bool fl = ((int)(e.deathTimer*24)) % 2 == 0;
    Color dc = fl ? Color{255,255,255,(unsigned char)(p*220)}
                  : Color{e.col.r,e.col.g,e.col.b,(unsigned char)(p*160)};
    DrawRectangleRounded({e.x+(ew-nw)*.5f, e.y+(eh-nh), nw, nh}, 0.4f, 4, dc);
}

// ── Enemy drawing ─────────────────────────────────────────────────────────────
void drawEnemy(const Enemy& e) {
    float ew = (e.type==3) ? 48.f : 28.f, eh = (e.type==3) ? 60.f : 28.f;
    if (e.type==3 && e.health==1 && ((int)(GetTime()*8))%2==0) return;

    float bounce = 0.f;
    if (e.type==0 || e.type==2) bounce = sinf(g.menuTimer*3.f + e.x*.01f) * 1.2f;
    if (e.type==4 && e.vy > 4.f) bounce = 2.f;

    DrawEllipse((int)(e.x+ew/2), (int)(e.y+eh+bounce), (int)(ew*.45f), 3, {0,0,0,55});

    const Color hp  = e.col;
    const Color hp2 = {(unsigned char)(e.col.r*4/5), (unsigned char)(e.col.g*4/5),
                       (unsigned char)(e.col.b*4/5), 255};
    float cx = e.x + ew * 0.5f;
    float cy = e.y + eh * 0.5f + bounce;

    if (e.type == 0) {
        float r = ew * 0.5f;
        float pd = e.vx >= 0 ? 1.f : -1.f;
        DrawCircle((int)cx, (int)cy, (int)(r*1.65f), {hp.r,hp.g,hp.b, 28});
        DrawCircle((int)cx, (int)cy, (int)(r*1.32f), {hp.r,hp.g,hp.b, 50});
        if (g.currentStage == 1) {
            // Stage 1: circle chaser
            DrawCircle((int)cx, (int)cy, (int)r, hp);
            DrawCircle((int)cx, (int)(cy - r*0.2f), (int)(r*0.65f), {hp.r,hp.g,hp.b,140});
            DrawLineEx({cx-8.f, cy-r*0.45f+1.f}, {cx-1.f, cy-r*0.62f}, 2.f, WHITE);
            DrawLineEx({cx+1.f, cy-r*0.62f},     {cx+8.f, cy-r*0.45f+1.f}, 2.f, WHITE);
            DrawCircle((int)(cx-5.f*pd), (int)(cy-3.f), 3, WHITE);
            DrawCircle((int)(cx+5.f*pd), (int)(cy-3.f), 3, WHITE);
            DrawCircle((int)(cx-5.f*pd+pd), (int)(cy-2.f), 1, {10,0,20,255});
            DrawCircle((int)(cx+5.f*pd+pd), (int)(cy-2.f), 1, {10,0,20,255});
            float wk = sinf(g.menuTimer*6.f + e.x*.02f) * 3.f;
            DrawCircle((int)(cx-5.f+wk), (int)(e.y+eh+bounce-2.f), 4, hp2);
            DrawCircle((int)(cx+5.f-wk), (int)(e.y+eh+bounce-2.f), 4, hp2);
        } else if (g.currentStage == 2) {
            // Stage 2: crystal octagon
            float spin = fmodf(g.menuTimer * 8.f + e.x * .1f, 45.f);
            DrawPoly({cx, cy}, 8, r, spin, hp);
            DrawPoly({cx, cy}, 8, r*0.56f, spin+22.5f, hp2);
            DrawCircle((int)(cx-5.f*pd), (int)(cy-3.f), 3, WHITE);
            DrawCircle((int)(cx+5.f*pd), (int)(cy-3.f), 3, WHITE);
            DrawCircle((int)(cx-5.f*pd+pd), (int)(cy-2.f), 1, {0,20,20,255});
            float wk = sinf(g.menuTimer*6.f + e.x*.02f) * 3.f;
            for (int ci = 0; ci < 3; ci++) {
                float fx = cx - 5.f + ci*5.f + wk*(ci%2==0?1.f:-1.f);
                DrawLineEx({fx, e.y+eh+bounce-3.f}, {fx+(ci%2==0?3.f:-3.f), e.y+eh+bounce+5.f}, 1.8f, hp2);
            }
        } else {
            // Stage 3: volcanic triangle pointing in movement direction
            float ang = e.vx >= 0 ? -90.f : 90.f;
            DrawPoly({cx, cy}, 3, r, ang, hp);
            DrawPoly({cx, cy}, 3, r*0.52f, ang+60.f, hp2);
            float pulse = 0.5f+0.5f*sinf(g.menuTimer*8.f+e.x*.02f);
            DrawCircle((int)(cx-3.f*pd), (int)(cy-2.f), 3, {255,200,50,(unsigned char)(150+pulse*105)});
            DrawCircle((int)(cx+3.f*pd), (int)(cy-2.f), 3, {255,200,50,(unsigned char)(100+pulse*80)});
            for (int di = 0; di < 3; di++) {
                float dx = cx + (di-1)*5.f;
                float dlen = 4.f + 3.f*sinf(g.menuTimer*5.f+di*1.2f);
                DrawLineEx({dx, e.y+eh+bounce-2.f}, {dx+(di-1)*1.5f, e.y+eh+bounce+dlen}, 1.8f, {255,100,10,140});
            }
        }

    } else if (e.type == 1) {
        float r = ew * 0.56f;
        DrawCircle((int)cx, (int)cy, (int)(r*1.6f), {hp.r,hp.g,hp.b, 28});
        DrawCircle((int)cx, (int)cy, (int)(r*1.3f), {hp.r,hp.g,hp.b, 50});
        if (g.currentStage == 1) {
            // Stage 1: slowly spinning diamond
            float angle = 45.f + fmodf(g.menuTimer * 22.f, 360.f);
            DrawPoly({cx, cy}, 4, r, angle, hp);
            DrawPoly({cx, cy}, 4, r*0.55f, angle+15.f, hp2);
            float eyeOff = e.vx >= 0 ? 3.f : -3.f;
            DrawCircle((int)(cx+eyeOff), (int)(cy-2.f), 3, WHITE);
            DrawCircle((int)(cx+eyeOff+(e.vx>=0?1.f:-1.f)), (int)(cy-1.f), 1, {10,0,20,255});
        } else if (g.currentStage == 2) {
            // Stage 2: hexagon spinning faster + crystal inner ring
            float angle = fmodf(g.menuTimer * 45.f + e.x * .1f, 360.f);
            DrawPoly({cx, cy}, 6, r, angle, hp);
            DrawPoly({cx, cy}, 6, r*0.52f, angle+30.f, hp2);
            float pulse = 0.5f+0.5f*sinf(g.menuTimer*6.f);
            DrawCircle((int)cx, (int)cy, (int)(r*0.28f), {hp.r,hp.g,hp.b,(unsigned char)(pulse*180+30)});
            float eyeOff = e.vx >= 0 ? 3.f : -3.f;
            DrawCircle((int)(cx+eyeOff), (int)(cy-2.f), 3, WHITE);
            DrawCircle((int)(cx+eyeOff+(e.vx>=0?1.f:-1.f)), (int)(cy-1.f), 1, {0,20,20,255});
        } else {
            // Stage 3: triangle spinning fast, fire-star effect
            float angle = fmodf(g.menuTimer * 80.f + e.x * .15f, 360.f);
            DrawPoly({cx, cy}, 3, r,       angle,        hp);
            DrawPoly({cx, cy}, 3, r*0.62f, angle+180.f,  hp2);
            float pulse = 0.5f+0.5f*sinf(g.menuTimer*9.f);
            DrawCircle((int)cx, (int)cy, (int)(r*0.32f), {255,200,50,(unsigned char)(pulse*200+30)});
            float eyeOff = e.vx >= 0 ? 3.f : -3.f;
            DrawCircle((int)(cx+eyeOff), (int)(cy-2.f), 3, {255,220,80,220});
            DrawCircle((int)(cx+eyeOff+(e.vx>=0?1.f:-1.f)), (int)(cy-1.f), 1, {80,20,0,255});
        }

    } else if (e.type == 2) {
        float r          = ew * 0.52f;
        bool  shootRight = g.player.x > e.x;
        DrawCircle((int)cx, (int)cy, (int)(r*1.65f), {hp.r,hp.g,hp.b, 28});
        DrawCircle((int)cx, (int)cy, (int)(r*1.32f), {hp.r,hp.g,hp.b, 50});
        float pulse    = 0.5f + 0.5f*sinf(g.menuTimer*7.f - e.shootTimer*4.f);
        float barrelLen = 14.f;
        float barrelX   = shootRight ? cx+r*0.82f : cx-r*0.82f-barrelLen;
        float tipX      = shootRight ? barrelX+barrelLen : barrelX;
        if (g.currentStage == 1) {
            // Stage 1: hexagon body
            DrawPoly({cx, cy}, 6, r,       0.f,  hp);
            DrawPoly({cx, cy}, 6, r*0.62f, 30.f, hp2);
            fillRect(barrelX, cy-3.f, barrelLen, 6.f, {50,0,30,255});
            DrawCircle((int)tipX, (int)cy, 4, {255,0,110,(unsigned char)(pulse*200+55)});
        } else if (g.currentStage == 2) {
            // Stage 2: diamond (4-gon) body angled + crystal tip
            float spin = fmodf(g.menuTimer * 15.f, 360.f);
            DrawPoly({cx, cy}, 4, r, spin,       hp);
            DrawPoly({cx, cy}, 4, r*0.55f, spin+45.f, hp2);
            fillRect(barrelX, cy-3.f, barrelLen, 5.f, {0,40,50,240});
            DrawCircle((int)tipX, (int)cy, 5, {0,255,255,(unsigned char)(pulse*220+35)});
            DrawCircle((int)tipX, (int)cy, 3, {255,255,255,(unsigned char)(pulse*160)});
        } else {
            // Stage 3: triangle body pointing at player + lava ember tip
            float ang = shootRight ? -30.f : 150.f;
            DrawPoly({cx, cy}, 3, r,        ang,       hp);
            DrawPoly({cx, cy}, 3, r*0.52f,  ang+60.f,  hp2);
            fillRect(barrelX, cy-3.f, barrelLen, 5.f, {60,20,0,240});
            DrawCircle((int)tipX, (int)cy, 5, {255,120,0,(unsigned char)(pulse*220+35)});
            DrawCircle((int)tipX, (int)cy, 3, {255,255,100,(unsigned char)(pulse*180)});
        }

    } else if (e.type == 3) {
        // Boss — large circle with spike crown and health bar
        int   maxH = 2 + g.currentStage / 2;
        float r    = ew * 0.48f;
        float aura = 0.5f + 0.5f*sinf(g.menuTimer*2.5f);
        DrawCircle((int)cx,(int)cy,(int)(r*1.9f+aura*8.f),{hp.r,hp.g,hp.b,(unsigned char)(15+aura*20)});
        DrawCircle((int)cx,(int)cy,(int)(r*1.5f+aura*5.f),{hp.r,hp.g,hp.b,(unsigned char)(28+aura*28)});
        DrawCircle((int)cx, (int)cy, (int)r, hp);
        DrawCircle((int)cx, (int)(cy-r*0.15f), (int)(r*0.65f), {255,80,160,120});
        // Spike crown across top arc
        for (int i = 0; i < 5; i++) {
            float sa     = -PI*0.78f + (float)i/4.f*PI*0.56f;
            float tipLen = (i%2==0) ? r*0.52f : r*0.34f;
            float bx1    = cx + cosf(sa-0.17f)*r*0.86f, by1 = cy + sinf(sa-0.17f)*r*0.86f;
            float bx2    = cx + cosf(sa+0.17f)*r*0.86f, by2 = cy + sinf(sa+0.17f)*r*0.86f;
            float tx     = cx + cosf(sa)*(r+tipLen),     ty  = cy + sinf(sa)*(r+tipLen);
            DrawTriangle({bx1,by1},{bx2,by2},{tx,ty},{255,200,0,255});
        }
        DrawCircle((int)(cx-7.f),(int)(cy-4.f),6,WHITE);
        DrawCircle((int)(cx+7.f),(int)(cy-4.f),6,WHITE);
        DrawCircle((int)(cx-5.f),(int)(cy-3.f),3,{10,0,20,255});
        DrawCircle((int)(cx+9.f),(int)(cy-3.f),3,{10,0,20,255});
        fillRect(e.x-2.f, e.y-16.f+bounce, ew+4.f,  9.f, {40,40,40,220});
        fillRect(e.x-2.f, e.y-16.f+bounce, (ew+4.f)*(float)e.health/maxH, 9.f, hp);
        DrawRectangleLinesEx({e.x-2.f,e.y-16.f+bounce,ew+4.f,9.f},1.f,{200,200,200,150});
        if (e.flashTimer > 0.f) {
            float ft = e.flashTimer / (2.f/60.f);
            DrawCircle((int)cx,(int)cy,(int)r,{255,255,255,(unsigned char)(ft*190)});
        }

    } else if (e.type == 4) {
        bool  grnd = (e.vy == 0.f);
        float sqsh = grnd ? 1.08f : 0.85f;
        float sqY  = eh * (1.f - sqsh);
        float r    = ew * 0.46f;
        DrawCircle((int)cx,(int)cy,(int)(r*1.65f),{hp.r,hp.g,hp.b, 28});
        DrawCircle((int)cx,(int)cy,(int)(r*1.32f),{hp.r,hp.g,hp.b, 50});
        if (g.currentStage == 1) {
            // Stage 1: rounded rect + spring coils
            if (grnd) {
                for (int i = 0; i < 4; i++) {
                    float zx = e.x + 5.f + i*5.f;
                    fillRect(zx, e.y+eh-10.f+i*2.f+bounce, 3.f, 6.f, {200,0,80,(unsigned char)(i%2==0?200:140)});
                }
            } else {
                for (int i = 0; i < 5; i++) {
                    float x1 = i%2==0 ? e.x+4.f : e.x+16.f;
                    float x2 = i%2==0 ? e.x+16.f: e.x+4.f;
                    DrawLineEx({x1,e.y+eh-14.f+i*3.f+bounce},{x2,e.y+eh-14.f+(i+1)*3.f+bounce},2.5f,{200,0,80,180});
                }
            }
            DrawRectangleRounded({e.x,e.y+sqY+bounce,ew,eh*sqsh},0.35f,5,hp);
            DrawRectangleRounded({e.x+2.f,e.y+sqY+2.f+bounce,ew-4.f,8.f*sqsh},0.5f,4,{255,80,160,120});
            float eyeX2 = e.vx >= 0 ? e.x+ew-9.f : e.x+5.f;
            DrawCircle((int)eyeX2,(int)(e.y+sqY+8.f+bounce),4,WHITE);
            DrawCircle((int)(eyeX2+(e.vx>=0?2.f:-2.f)),(int)(e.y+sqY+9.f+bounce),2,{10,0,20,255});
        } else if (g.currentStage == 2) {
            // Stage 2: hexagon body + crystal shard legs
            float spin = fmodf(g.menuTimer * 12.f + e.x*.1f, 60.f);
            DrawPoly({cx, cy+sqY*0.5f}, 6, r*sqsh, spin, hp);
            DrawPoly({cx, cy+sqY*0.5f}, 6, r*sqsh*0.55f, spin+30.f, hp2);
            for (int i = 0; i < 4; i++) {
                float sx = e.x + 4.f + i*6.f;
                float slen = grnd ? 7.f : 3.f + 4.f*sinf(g.menuTimer*8.f+i);
                DrawLineEx({sx, e.y+eh-6.f+bounce}, {sx+(i%2==0?2.f:-2.f), e.y+eh+slen+bounce}, 2.f, hp2);
                DrawCircle((int)(sx+(i%2==0?2.f:-2.f)), (int)(e.y+eh+slen+bounce), 2, {255,255,255,120});
            }
            float eyeX2 = e.vx >= 0 ? cx+4.f : cx-4.f;
            DrawCircle((int)eyeX2,(int)(cy+sqY*0.5f-3.f),3,WHITE);
            DrawCircle((int)(eyeX2+(e.vx>=0?1.f:-1.f)),(int)(cy+sqY*0.5f-2.f),1,{0,20,20,255});
        } else {
            // Stage 3: triangle body + lava drip legs
            float ang = grnd ? -90.f : -60.f;
            DrawPoly({cx, cy+sqY*0.5f}, 3, r*sqsh,        ang,       hp);
            DrawPoly({cx, cy+sqY*0.5f}, 3, r*sqsh*0.52f,  ang+60.f,  hp2);
            for (int i = 0; i < 3; i++) {
                float dx = cx + (i-1)*6.f;
                float dlen = grnd ? 8.f+2.f*sinf(g.menuTimer*4.f+i) : 3.f;
                DrawLineEx({dx, e.y+eh-4.f+bounce}, {dx+(i-1)*1.5f, e.y+eh+dlen+bounce}, 2.f, {255,100,10,160});
                if (grnd) DrawCircle((int)(dx+(i-1)*1.5f),(int)(e.y+eh+dlen+bounce),3,{255,60,0,120});
            }
            float eyeX2 = e.vx >= 0 ? cx+4.f : cx-4.f;
            DrawCircle((int)eyeX2,(int)(cy+sqY*0.5f-3.f),3,{255,220,80,220});
            DrawCircle((int)(eyeX2+(e.vx>=0?1.f:-1.f)),(int)(cy+sqY*0.5f-2.f),1,{80,20,0,255});
        }
    }
}

// ── World items ───────────────────────────────────────────────────────────────
void drawHeart(float x, float y, Color c) {
    fillRect(x+2,y,4,2,c); fillRect(x+8,y,4,2,c); fillRect(x,y+2,14,2,c);
    fillRect(x+2,y+4,10,2,c); fillRect(x+4,y+6,6,2,c); fillRect(x+6,y+8,2,2,c);
}

void drawPlatform(const Platform& p) {
    // Vanishing platform: flicker when close to vanishing, invisible when phased out
    if (p.canVanish) {
        if (p.vanishTimer >= 1.8f) return; // fully gone
        if (p.vanishTimer >= 1.2f) {
            // Flicker warning
            if (((int)(p.vanishTimer * 14.f)) % 2 == 0) return;
        }
    }
    bool mov = (p.vx || p.vy);
    int th = stageTheme(g.currentStage);
    int ti = mov ? 3 : (th==3?2 : th==2?1 : th==4?4 : th==5?5 : th==6?6 : 0);
    Texture2D& tx = tileTex[ti];
    for (float tx2 = p.x; tx2 < p.x+p.w; tx2 += TILE) {
        float tw = std::min((float)TILE, p.x+p.w-tx2);
        for (float ty = p.y; ty < p.y+p.h; ty += TILE) {
            float th = std::min((float)TILE, p.y+p.h-ty);
            float srcY = (ty == p.y) ? 0.f : (TILE-th > 2 ? TILE-th : 10.f);
            if (p.h <= 20 && ty == p.y) srcY = 0.f;
            DrawTextureRec(tx, {0.f, srcY, tw, th}, {tx2, ty}, WHITE);
        }
    }
    if (!mov && (th==1||th==2) && p.h > 20) {
        Color blade  = {(unsigned char)(60+(th==2?-20:0)), 220, 65, 255};
        Color bladeD = {(unsigned char)(35+(th==2?-15:0)), 160, 38, 255};
        for (float bx = p.x+5; bx < p.x+p.w-4; bx += 7) {
            float sway = sinf(g.menuTimer*1.4f + bx*.25f) * 1.8f;
            float bh   = 4.f + sinf(bx*.8f) * 2.f;
            DrawLineEx({bx, (float)p.y}, {bx+sway, p.y-bh}, 1.5f, (int)(bx/7)%2==0 ? blade : bladeD);
        }
    }
    if (!mov && (th==3||th==6) && p.h <= 20) {
        float gl = 0.5f + 0.5f * sinf(g.menuTimer*2.5f + p.x*.01f);
        Color rune = th==6 ? Color{60,40,140,(unsigned char)(gl*80+25)}
                           : Color{160,80,220,(unsigned char)(gl*80+30)};
        DrawCircle((int)(p.x+p.w/2), (int)(p.y+p.h/2), 4, rune);
    }
    if (!mov && th==4 && p.h <= 20) {
        float gl = 0.5f + 0.5f * sinf(g.menuTimer*3.f + p.x*.01f);
        DrawCircle((int)(p.x+p.w/2), (int)(p.y+p.h/2), 4,
            Color{220,80,15,(unsigned char)(gl*70+25)});
    }
    if (!mov && th==5 && p.h <= 20) {
        float gl = 0.5f + 0.5f * sinf(g.menuTimer*2.f + p.x*.01f);
        DrawCircle((int)(p.x+p.w/2), (int)(p.y+p.h/2), 3,
            Color{160,210,255,(unsigned char)(gl*60+20)});
    }
    if (mov) {
        Color glow = {140,215,255,(unsigned char)(60+sinf(g.menuTimer*3.f+p.x*.01f)*30)};
        DrawRectangleLinesEx({p.x-1, p.y-1, p.w+2, p.h+2}, 1.5f, glow);
    }
}

void drawMushroom(const Mushroom& m) {
    float bob = sinf(g.menuTimer*2.2f + m.x*.01f) * 2.5f;
    fillRect(m.x-4, m.y+2+bob, 12, 12, {230,185,140,255});
    DrawRectangleRounded({m.x-11, m.y-12+bob, 26, 16}, 0.5f, 6, {192,57,43,255});
    fillRect(m.x-5, m.y-6+bob, 26, 4, {192,57,43,255});
    DrawCircle((int)m.x-2, (int)(m.y-7+bob), 3, WHITE);
    DrawCircle((int)m.x+7, (int)(m.y-9+bob), 2, WHITE);
    DrawCircle((int)m.x-7, (int)(m.y-5+bob), 2, WHITE);
    float gl = 0.5f + 0.5f * sinf(g.menuTimer*3.f);
    drawBloom(m.x, m.y+bob, 12.f, {255,190,11,(unsigned char)(gl*55+22)});
}

void drawCheckpoint(const Checkpoint& cp) {
    fillRect(cp.x, cp.y-36, 5, 41, {140,140,150,255});
    Color flagC = cp.activated ? Color{39,174,96,255} : Color{180,180,180,255};
    DrawTriangle({cp.x+5, cp.y-34}, {cp.x+5, cp.y-16}, {cp.x+22, cp.y-25}, flagC);
    if (cp.activated) {
        float gl = 0.5f + 0.5f * sinf(g.menuTimer*4.f);
        drawBloom(cp.x+2.f, cp.y-25.f, 10.f, {39,174,96,(unsigned char)(gl*65+25)});
    }
}

void drawSpike(const Spike& s) {
    int th = stageTheme(g.currentStage);
    // Base crystal colors per theme
    Color tip  = (th == 3) ? Color{180,80,255,255} : (th == 4) ? Color{255,80,20,255} : Color{120,220,255,255};
    Color base = (th == 3) ? Color{80,20,120,255}  : (th == 4) ? Color{120,30,5,255}  : Color{40,80,140,255};
    float gl = 0.5f + 0.5f * sinf(g.menuTimer * 3.f + s.x * 0.02f);
    // Glow under spikes
    DrawEllipse((int)(s.x + s.w/2), (int)s.y, (int)(s.w * 0.7f), 4,
        Color{tip.r, tip.g, tip.b, (unsigned char)(gl * 55 + 15)});
    // Triangle teeth
    int teeth = std::max(1, (int)(s.w / 10.f));
    float tw = s.w / teeth;
    for (int i = 0; i < teeth; i++) {
        float bx1 = s.x + i * tw;
        float bx2 = s.x + (i + 1) * tw;
        float tx  = s.x + (i + 0.5f) * tw;
        float h   = 10.f + 4.f * ((i % 2 == 0) ? 1.f : 0.f);
        Color tc  = i % 2 == 0 ? tip : Color{(unsigned char)((tip.r+base.r)/2), (unsigned char)((tip.g+base.g)/2), (unsigned char)((tip.b+base.b)/2), 255};
        DrawTriangle({bx1, s.y}, {bx2, s.y}, {tx, s.y - h}, tc);
        // Highlight edge
        DrawLineEx({tx, s.y - h}, {bx1, s.y}, 1.f, {255,255,255,(unsigned char)(gl*80)});
    }
}

void drawGeyser(const Geyser& geo) {
    const float GY = 400.f;
    float t = g.menuTimer;
    float warmup = 0.f;
    if (!geo.firing) {
        warmup = std::max(0.f, 1.f - geo.cooldown / std::max(geo.maxCooldown, 0.01f));
        // Warning glow when about to fire
        if (warmup > 0.3f) {
            float pulse = 0.5f + 0.5f * sinf(t * 12.f);
            unsigned char wa = (unsigned char)((warmup - 0.3f) / 0.7f * (80 + pulse * 80));
            DrawCircle((int)geo.x, (int)GY, (int)(14 + pulse * 6), {255,140,20,wa});
            DrawCircle((int)geo.x, (int)GY, 5, {255,200,80,(unsigned char)(wa * 1.5f < 255 ? wa * 1.5f : 255)});
        }
        return;
    }
    // Firing — animated fire column
    float phase = 1.f - geo.fireTimer / 0.7f; // 0→1 as fire progresses
    float colH  = 110.f * std::min(1.f, phase * 3.f);
    float colW  = 28.f + sinf(t * 18.f) * 4.f;
    // Core column
    for (int row = 0; row < (int)colH; row += 3) {
        float y   = GY - row - 3.f;
        float xwobble = sinf(t * 24.f + row * 0.15f) * (3.f * (1.f - row / colH));
        float aw  = colW * (0.6f + 0.4f * (1.f - (float)row / colH));
        unsigned char fa = (unsigned char)(220 * (1.f - (float)row / (colH + 10.f)));
        Color fc = row < colH * 0.4f ? Color{255,100,20,fa} : Color{255,200,50,fa};
        DrawEllipse((int)(geo.x + xwobble), (int)y, (int)(aw / 2), 5, fc);
    }
    // Particles at tip
    for (int i = 0; i < 4; i++) {
        float ang = t * 8.f + i * 1.6f;
        float r = 8.f + sinf(ang) * 4.f;
        DrawCircle((int)(geo.x + cosf(ang) * r), (int)(GY - colH + sinf(ang * 0.5f) * 5.f),
            3, {255,220,80,(unsigned char)(120 + sinf(ang + i) * 80)});
    }
    // Base glow at floor
    DrawEllipse((int)geo.x, (int)GY, 20, 5, {255,100,10,180});
}

void drawFlag() {
    bool active = true;
    for (const auto& e : g.enemies) if (e.type==3 && e.alive) active = false;
    float fx = g.levelWidth - 80.f, glow = 0.5f + 0.5f*sinf(g.menuTimer*4.f);
    DrawRectangle((int)fx+4, 281, 4, 125, {0,0,0,40});
    if (active) {
        drawBloom(fx+3.f, 295.f, 14.f+glow*4.f, {39,174,96,(unsigned char)(45+glow*45)});
    }
    fillRect(fx, 280, 6, 125, {149,165,166,255});
    Color flagC = active ? Color{39,174,96,255} : Color{190,190,190,255};
    DrawTriangle({fx+6, 310.f}, {fx+6, 280.f}, {fx+44, 295.f}, flagC);
    DrawTriangle({fx+6, 282.f}, {fx+6, 292.f}, {fx+44, 287.f}, {255,255,255,60});
    fillRect(fx-8, 400, 28, 10, {100,110,115,255});
    if (active) {
        char lbl[16];
        if (g.levelInStage < 10) snprintf(lbl, sizeof(lbl), "LVL %d >", g.levelInStage+1);
        else                     snprintf(lbl, sizeof(lbl), "STAGE END!");
        DrawText(lbl, (int)fx-16, 265, 10, {255,255,255,200});
    }
}

// ── Background layers ─────────────────────────────────────────────────────────
static Color skyTop() {
    int th = stageTheme(g.currentStage);
    if (th==1) return { 18,  0, 42, 255};
    if (th==2) return { 15,  2, 40, 255};
    if (th==3) return { 10,  0, 26, 255};
    if (th==4) return { 38,  0, 36, 255};
    if (th==5) return { 15,  5, 48, 255};
    return {26,  0, 51, 255};
}
static Color skyBot() {
    int th = stageTheme(g.currentStage);
    if (th==1) return { 10,  0, 26, 255};
    if (th==2) return {  8,  1, 24, 255};
    if (th==3) return {  4,  0, 14, 255};
    if (th==4) return { 22,  0, 22, 255};
    if (th==5) return {  8,  3, 32, 255};
    return {13,  0, 31, 255};
}

static void drawClouds(float cx) {
    Color cc = {255,255,255,(unsigned char)(g.currentLevel==3?70:210)};
    float tot = SW + 250.f;
    for (int i = 0; i < 5; i++) {
        const auto& c = CLOUDS[i];
        float raw = fmodf(fmodf(c.cx - cx*.3f, tot) + tot, tot), s = raw - 125.f;
        DrawEllipse((int)s, (int)c.cy, c.cw, c.ch, cc);
        DrawEllipse((int)(s+c.cw*.45f), (int)(c.cy-c.ch*.4f), c.cw*.65f, c.ch*.75f, cc);
        DrawEllipse((int)(s-c.cw*.35f), (int)(c.cy-c.ch*.2f), c.cw*.55f, c.ch*.6f,  cc);
        DrawEllipse((int)s, (int)(c.cy+c.ch*.3f), c.cw, c.ch*.4f, {0,0,0,(unsigned char)(g.currentLevel==3?20:40)});
    }
}

static void drawStars(float cx) {
    for (int i = 0; i < 45; i++) {
        float base = fmodf((float)(i*367), 4800.f);
        float sx = fmodf(fmodf(base - cx*.05f, (float)SW+200.f) + (float)SW+200.f, (float)SW+200.f) - 100.f;
        float sy = 4.f + fmodf((float)(i*97), 275.f);
        float br = 0.6f + 0.4f * sinf(g.menuTimer*1.8f + i*.9f);
        int r = (i%7==0) ? 3 : (i%3==0 ? 2 : 1);
        DrawCircle((int)sx, (int)sy, r, {255,255,255,(unsigned char)(br*200+40)});
        if (i%7==0) {
            Color sp = {255,255,220,(unsigned char)(br*80)};
            DrawLineEx({sx-4,sy},{sx+4,sy},1.f,sp);
            DrawLineEx({sx,sy-4},{sx,sy+4},1.f,sp);
        }
    }
}

static void drawHorizonSun(float cx) {
    float horizY = (float)SH * 0.56f;
    float rawX   = (float)SW * 0.5f - cx * 0.05f;
    float sunX   = fmodf(fmodf(rawX, (float)SW + 200.f) + (float)SW + 200.f, (float)SW + 200.f) - 100.f;
    float sunR   = 58.f;
    int   th     = stageTheme(g.currentStage);
    Color sunC   = th==4 ? Color{255, 60, 10,255}
                 : th==5 ? Color{ 80,190,255,255}
                 :         Color{255, 50,170,255};
    // Glow halos
    for (int i = 7; i > 0; i--) {
        Color gc = sunC; gc.a = (unsigned char)(5 * i);
        DrawCircle((int)sunX, (int)horizY, (int)(sunR + i * 14.f), gc);
    }
    // Filled circle
    DrawCircle((int)sunX, (int)horizY, (int)sunR, sunC);
    // Horizontal scanline gaps
    Color gap = skyBot();
    for (int s = 1; s < 9; s++) {
        float sy  = horizY + (s / 9.f) * sunR;
        float hw  = sqrtf(std::max(0.f, sunR*sunR - (sy - horizY)*(sy - horizY)));
        DrawRectangle((int)(sunX - hw), (int)sy, (int)(hw * 2.f + 1.f), 3, gap);
    }
}

static void drawPerspectiveGrid(float cx) {
    const float horizY = (float)SH * 0.56f;
    const float vpX    = (float)SW * 0.5f;
    const Color gc     = {0, 220, 185, 22};
    // Horizontal lines — non-linear, bunched near horizon
    for (int i = 1; i <= 14; i++) {
        float frac = (float)i / 14.f;
        float y    = horizY + frac * frac * ((float)SH - horizY);
        unsigned char la = (unsigned char)(gc.a * (0.25f + frac * 0.75f));
        DrawLine(0, (int)y, SW, (int)y, {gc.r, gc.g, gc.b, la});
    }
    // Converging vertical lines
    for (int i = 0; i <= 10; i++) {
        float frac = (float)i / 10.f;
        float botX = frac * (float)SW;
        float dist = fabsf(frac - 0.5f) * 2.f;
        unsigned char la = (unsigned char)(gc.a * (1.f - dist * 0.6f));
        DrawLineEx({vpX, horizY}, {botX, (float)SH}, 0.7f, {gc.r, gc.g, gc.b, la});
    }
}

static void drawTrees(float cx) {
    static const float TX[] = {180,440,680,950,1200,1480,1730,2000,2250,2500};
    static const float TH[] = {88,72,100,82,95,78,92,75,98,84};
    static const float TW[] = {54,44,62,50,58,46,56,44,60,52};
    float tot    = (float)SW + 300.f;
    float ground = (float)SH - 62.f;
    int   th     = stageTheme(g.currentStage);
    Color trunk  = {22,  6, 40, 220};
    Color tipC   = th==4 ? Color{ 90, 15,  5, 180}
                 : th==5 ? Color{ 50, 70,150, 170}
                 :         Color{ 60, 15, 90, 180};
    for (int i = 0; i < 10; i++) {
        float sx = fmodf(fmodf(TX[i] - cx * .38f, tot) + tot, tot) - 80.f;
        if (sx < -100 || sx > SW + 80) continue;
        float h = TH[i], w = TW[i];
        fillRect(sx + w*.36f, ground - h*.5f,  w*.28f, h*.52f, trunk);
        DrawTriangle({sx+w*.05f, ground-h*.42f}, {sx+w*.95f, ground-h*.42f},
                     {sx+w*.5f,  ground-h},       trunk);
        DrawTriangle({sx+w*.18f, ground-h*.72f}, {sx+w*.82f, ground-h*.72f},
                     {sx+w*.5f,  ground-h},       tipC);
    }
}

static void drawLava() {
    float t = (float)GetTime();
    fillRect(0, SH-20, (float)SW, 20, {180,50,10,255});
    for (int i = 0; i < 20; i++) {
        float lx = fmodf(i*97.3f + t*18.f, (float)SW);
        float ly = SH-20.f + sinf(t*4.f + i*1.2f) * 4.f;
        DrawCircle((int)lx, (int)ly, 6+(i%3)*3, {240,130,20,200});
    }
    for (int i = 0; i < 15; i++) {
        float px = fmodf(i*131.7f + t*25.f, (float)SW);
        float py = fmodf(i*73.f   + t*40.f, (float)(SH+60)) - 60.f;
        unsigned char a = (unsigned char)(80+((int)(t*5+i)%3)*40);
        DrawCircle((int)px, (int)(SH-24-(int)fmodf(py<0?py+SH:py,SH-24)), 2, {220,100,20,a});
    }
}

static void drawSnow() {
    float t = (float)GetTime();
    for (int i = 0; i < 60; i++) {
        float sx = fmodf(i*173.f + t*20.f - (float)(i%3)*8.f, (float)SW);
        float sy = fmodf(i*89.f  + t*55.f, (float)(SH+20)) - 10.f;
        DrawCircle((int)sx, (int)sy, 1+(i%3), {220,235,255,(unsigned char)(80+(i%4)*35)});
    }
}

static void drawRain() {
    float t = (float)GetTime();
    for (int i = 0; i < 90; i++) {
        float rx = fmodf(i*173.7f + t*60.f,  (float)SW);
        float ry = fmodf(i*91.3f  + t*250.f, (float)(SH+40)) - 20.f;
        float len = 7.f + (float)(i%4) * 2.f;
        DrawLineEx({rx, ry}, {rx-2.f, ry+len}, 1.f, {140,170,220,(unsigned char)(70+(i%3)*25)});
    }
}

static void drawVignette() {
    for (int i = 0; i < 22; i++) {
        DrawRectangleLinesEx({(float)i,(float)i,(float)(SW-i*2),(float)(SH-i*2)},
            2.f, {13,0,31,(unsigned char)((22-i)*4.5f)});
    }
}

// ── Settings screen ───────────────────────────────────────────────────────────
void drawSettings() {
    float t = g.menuTimer, rt = (float)GetTime();
    DrawRectangleGradientV(0,0,SW,SH,{5,6,18,255},{8,4,22,255});
    drawCyberGrid(t, 16);
    for (int i = 0; i < 20; i++) {
        float px2 = fmodf(i*311.f,(float)SW), py2 = fmodf(SH-(i*173.f+rt*(12.f+(i%5)*4.f)),(float)SH+10);
        float br = 0.5f+0.5f*sinf(rt*2.f+i*.8f);
        DrawCircle((int)px2,(int)py2,1,{0,255,200,(unsigned char)(br*100)});
    }
    const char* ttl = "// SETTINGS //";
    drawNeonText(ttl, SW/2-MeasureText(ttl,36)/2, 16, 36, {0,255,200,255});
    DrawLineEx({SW/2-110.f,60},{SW/2+110.f,60},1.f,{0,255,200,70});

    float panX=SW/2.f-200.f, panY=66.f, panW=400.f, panH=366.f;
    DrawRectangle((int)panX,(int)panY,(int)panW,(int)panH,{5,8,20,225});
    for (int i=3;i>0;i--) DrawRectangleLinesEx({panX-i,panY-i,panW+i*2,panH+i*2},1.f,{0,255,200,(unsigned char)(12/i)});
    DrawRectangleLinesEx({panX,panY,panW,panH},1.5f,{0,255,200,100});
    drawCornerBrackets(panX,panY,panW,panH,10.f,{0,255,200,160});

    float slX=panX+30.f, slW=280.f;
    Vector2 mp = g.mouse;
    auto drawSlider = [&](const char* lbl, float& val, float sy) {
        DrawText(lbl,(int)slX,(int)(sy-16),13,{0,255,200,190});
        DrawRectangle((int)slX,(int)sy,(int)slW,8,{12,18,32,220});
        DrawRectangleLinesEx({slX,sy,slW,8},1.f,{0,255,200,45});
        float fill = slW * val;
        if (fill > 1.f) { DrawRectangle((int)slX,(int)sy,(int)fill,8,{0,200,160,220});
                          DrawRectangle((int)(slX+fill-2),(int)sy,3,8,{0,255,200,255}); }
        float hx = slX+fill;
        bool ov = CheckCollisionPointCircle(mp,{hx,sy+4.f},10.f);
        for (int i=3;i>0;i--) DrawCircle((int)hx,(int)(sy+4),10+i,{0,255,200,(unsigned char)(12/i)});
        DrawCircle((int)hx,(int)(sy+4),ov?9:7,ov?Color{0,255,200,255}:Color{0,190,155,230});
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)&&CheckCollisionPointRec(mp,{slX-10,sy-14,slW+20,34}))
            val = std::max(0.f,std::min(1.f,(mp.x-slX)/slW));
        char vb[8]; snprintf(vb,sizeof(vb),"%d%%",(int)(val*100));
        DrawText(vb,(int)(slX+slW+12),(int)(sy-2),13,{0,255,200,170});
    };
    drawSlider("MASTER VOLUME",g.masterVolume,110);
    drawSlider("MUSIC VOLUME", g.musicVolume, 162);
    drawSlider("SFX VOLUME",   g.sfxVolume,   214);

    DrawText("HARD MODE",(int)slX,263,15,{0,255,200,210});
    const char* hmDesc = g.hardMode?"Faster enemies, shorter invincibility":"Standard difficulty";
    DrawText(hmDesc,(int)slX,281,11,g.hardMode?Color{255,80,80,180}:Color{80,130,140,140});
    if (drawBtn(SW/2+52,256,96,30,g.hardMode?"[ ON ]":"[OFF]",{5,8,18,220},{0,20,15,240},14)) g.hardMode=!g.hardMode;

    // Fullscreen toggle
    bool isFS = IsWindowFullscreen();
    DrawText("FULLSCREEN",(int)(panX+panW/2+8),263,15,{0,255,200,210});
    const char* fsDesc = isFS ? "Window: Fullscreen" : "Window: Normal";
    DrawText(fsDesc,(int)(panX+panW/2+8),281,11,isFS?Color{100,255,200,180}:Color{80,130,140,140});
    if (drawBtn(panX+panW-108,256,96,30,isFS?"[ ON ]":"[OFF]",{5,8,18,220},{0,20,15,240},14))
        ToggleFullscreen();

    // Controls / key rebind section
    DrawLineEx({panX+10,320},{panX+panW-10,320},1.f,{0,255,200,35});
    DrawText("// CONTROLS //", (int)(panX+10), 324, 11, {0,255,200,100});

    auto keyName = [](int k) -> const char* {
        switch(k) {
            case 263: return "LEFT";   case 262: return "RIGHT";  case 264: return "DOWN";
            case 265: return "UP";     case 32:  return "SPACE";  case 340: return "L-SHIFT";
            case 341: return "L-CTRL"; case 342: return "L-ALT";  case 257: return "ENTER";
            case 256: return "ESC";    case 259: return "BKSP";   case 258: return "TAB";
            case 65:  return "A";      case 66:  return "B";      case 67:  return "C";
            case 68:  return "D";      case 69:  return "E";      case 70:  return "F";
            case 71:  return "G";      case 72:  return "H";      case 73:  return "I";
            case 74:  return "J";      case 75:  return "K";      case 76:  return "L";
            case 77:  return "M";      case 78:  return "N";      case 79:  return "O";
            case 80:  return "P";      case 81:  return "Q";      case 82:  return "R";
            case 83:  return "S";      case 84:  return "T";      case 85:  return "U";
            case 86:  return "V";      case 87:  return "W";      case 88:  return "X";
            case 89:  return "Y";      case 90:  return "Z";
            default: { static char buf[10]; snprintf(buf,sizeof(buf),"#%d",k); return buf; }
        }
    };

    struct RRow { const char* lbl; int* key; int tgt; };
    RRow rrows[] = { {"MOVE LEFT",  &g.keyLeft,  1},
                     {"MOVE RIGHT", &g.keyRight, 2},
                     {"JUMP",       &g.keyJump,  3},
                     {"DASH",       &g.keyDash,  4} };
    for (int i = 0; i < 4; i++) {
        float ry = 338.f + i * 23.f;
        DrawText(rrows[i].lbl, (int)(panX+10), (int)ry, 12, {0,255,200,185});
        if (g.rebindTarget == rrows[i].tgt) {
            float wa = 0.5f+0.5f*sinf(rt*6.f);
            DrawText("PRESS A KEY...", (int)(panX+172), (int)ry, 12, {255,200,0,(unsigned char)(150+wa*100)});
        } else {
            char kbuf[24]; snprintf(kbuf,sizeof(kbuf),"[%s]", keyName(*rrows[i].key));
            int kw = MeasureText(kbuf,12);
            DrawText(kbuf,(int)(panX+268-kw),(int)ry,12,{255,215,0,200});
            if (drawBtn(panX+274,ry-2,70,17,"REBIND",{5,8,18,220},{0,20,15,240},10))
                g.rebindTarget = rrows[i].tgt;
        }
    }

    if (drawBtn(SW/2-76,434,152,28,"[ BACK ]",{5,8,18,220},{0,20,15,240},16)) {
        g.screen = (g.screenBeforeEdit == Screen::Pause) ? Screen::Pause : Screen::Menu;
        saveSettings(); applyVolumes();
    }
    DrawText("ESC to go back",SW/2-MeasureText("ESC to go back",11)/2,445,11,{0,255,200,70});
    drawScanlines(13);
    drawCloseBtn();
}

// ── Menu ──────────────────────────────────────────────────────────────────────
void drawMenu() {
    float t = g.menuTimer, rt = (float)GetTime();

    DrawRectangleGradientV(0,0,SW,SH,{5,6,18,255},{8,4,22,255});
    drawCyberGrid(t, 18);

    // Diagonal speed lines
    for (int i = 0; i < 6; i++) {
        float lx = fmodf(i*200.f + t*38.f, (float)(SW+220)) - 110.f;
        float la = 12.f + 10.f * sinf(t*1.3f + i*.9f);
        DrawLineEx({lx-28.f,0},{lx,(float)SH},0.6f,{0,255,200,(unsigned char)la});
    }
    // Floating neon sparks rising upward
    for (int i = 0; i < 40; i++) {
        float px2 = fmodf(i*241.f, (float)SW);
        float py2 = fmodf(SH - (i*127.f + rt*(14.f+(i%7)*4.f)), (float)SH+20);
        float br  = 0.5f + 0.5f * sinf(rt*2.2f + i*.7f);
        int   cr  = i%3;
        Color pc  = cr==0 ? Color{0,255,200,(unsigned char)(br*130)}
                  : cr==1 ? Color{255,215,0,(unsigned char)(br*95)}
                  :         Color{160,50,255,(unsigned char)(br*80)};
        DrawCircle((int)px2,(int)py2,i%6==0?2:1,pc);
        if (i%8==0) DrawLineEx({px2,py2},{px2,py2+5.f},0.7f,pc);
    }

    // Separator line
    float la2 = 0.45f+0.45f*sinf(t*1.8f);
    DrawLineEx({0,152},{(float)SW,152},0.8f,{0,255,200,(unsigned char)(la2*50)});

    // ── Title ──────────────────────────────────────────────────────────────────
    const char* title = "PLATFORMER"; int tlen = (int)strlen(title);
    int fontSize = 64, totalW = MeasureText(title,fontSize), startX = SW/2-totalW/2;
    bool doGlitch = fmodf(t, 5.f) > 4.55f;
    char lbuf[2]={0,0}; int oxl = startX;
    for (int i = 0; i < tlen; i++) {
        lbuf[0] = title[i]; int lw2 = MeasureText(lbuf,fontSize);
        float gyOff = (doGlitch && i%3==1) ? sinf(t*48.f+i)*5.f : 0.f;
        float gxOff = (doGlitch && i%3==2) ? cosf(t*36.f+i)*3.f : 0.f;
        for (int gi=4;gi>0;gi--) {
            unsigned char ga=(unsigned char)(22/gi);
            DrawText(lbuf,(int)(oxl+gxOff)-gi,(int)(54+gyOff),fontSize,{0,255,200,ga});
            DrawText(lbuf,(int)(oxl+gxOff)+gi,(int)(54+gyOff),fontSize,{0,255,200,ga});
        }
        if (doGlitch && i%4==0) {
            DrawText(lbuf,(int)(oxl+gxOff)+3,(int)(54+gyOff),fontSize,{255,30,80,45});
            DrawText(lbuf,(int)(oxl+gxOff)-3,(int)(54+gyOff),fontSize,{30,80,255,45});
        }
        Color lc = i%2==0 ? Color{210,255,245,255} : Color{0,255,200,255};
        DrawText(lbuf,(int)(oxl+gxOff),(int)(54+gyOff),fontSize,lc);
        oxl += lw2+1;
    }

    // Subtitle
    const char* sub = ">  3 STAGES  //  15 LEVELS  //  3 BOSS FIGHTS  //  SPEED RUN  <";
    float subA = 120.f + 80.f * sinf(t*0.9f);
    DrawText(sub, SW/2-MeasureText(sub,11)/2, 132, 11, {0,255,200,(unsigned char)subA});

    // High score panel
    if (g.highScore > 0) {
        std::string hs = "HIGH SCORE: " + std::to_string(g.highScore);
        int hw2 = MeasureText(hs.c_str(),14); float hpw=hw2+24.f, hpx=SW/2.f-hpw/2.f;
        DrawRectangle((int)hpx,158,(int)hpw,20,{8,10,25,210});
        DrawRectangleLinesEx({hpx,158,hpw,20},1.f,{255,215,0,80});
        DrawText(hs.c_str(),(int)(hpx+12),162,14,{255,215,0,235});
    }

    // Buttons
    if (drawBtn(SW/2-115,187,230,50,"START GAME",{5,8,18,220},{0,20,15,240},24)) resetGame();
    if (drawBtn(SW/2-92, 247,184,42,"SETTINGS",  {5,8,18,220},{0,20,15,240},20)) { g.screenBeforeEdit=Screen::Menu; g.screen=Screen::Settings; }
    if (drawBtn(SW/2-72, 298,144,40,"QUIT",       {5,8,18,220},{0,20,15,240},20)) CloseWindow();

    // Controls panel
    float cpY=348.f, cpW=356.f, cpX=SW/2.f-cpW/2.f;
    DrawRectangle((int)cpX,(int)cpY,(int)cpW,70,{5,8,20,185});
    DrawRectangleLinesEx({cpX,cpY,cpW,70},1.f,{0,255,200,40});
    drawCornerBrackets(cpX,cpY,cpW,70,7.f,{0,255,200,100});
    DrawText("[ CONTROLS ]",SW/2-MeasureText("[ CONTROLS ]",12)/2,(int)cpY+5,12,{0,255,200,180});
    DrawText("WASD/Arrows: Move  |  Space: Jump  |  Shift: Dash",
        SW/2-MeasureText("WASD/Arrows: Move  |  Space: Jump  |  Shift: Dash",11)/2,(int)cpY+21,11,{180,240,220,195});
    DrawText("Stomp from above  |  Double Jump  |  Wall Jump  |  ESC: Pause",
        SW/2-MeasureText("Stomp from above  |  Double Jump  |  Wall Jump  |  ESC: Pause",11)/2,(int)cpY+35,11,{180,240,220,185});
    DrawText("Reach the flag  |  F2: Level Editor  |  F11: Fullscreen",
        SW/2-MeasureText("Reach the flag  |  F2: Level Editor  |  F11: Fullscreen",11)/2,(int)cpY+49,11,{255,200,100,175});

    // Credit
    const char* tag = "// Built by Abdullah Javid //";
    DrawText(tag, SW/2-MeasureText(tag,11)/2, SH-16, 11, {0,255,200,80});

    drawScanlines(14);
    drawCloseBtn();
}

// ── Pause overlay ─────────────────────────────────────────────────────────────
void drawPause() {
    float rt = (float)GetTime();
    DrawRectangle(0,0,SW,SH,{0,0,5,175});
    drawScanlines(12);

    float pw=290.f, ph=300.f, px=SW/2.f-pw/2.f, py=SH/2.f-ph/2.f;
    DrawRectangle((int)px,(int)py,(int)pw,(int)ph,{5,8,20,248});
    for (int i=4;i>0;i--) DrawRectangleLinesEx({px-i,py-i,pw+i*2,ph+i*2},1.f,{0,255,200,(unsigned char)(16/i)});
    DrawRectangleLinesEx({px,py,pw,ph},1.5f,{0,255,200,185});
    drawCornerBrackets(px,py,pw,ph,12.f,{0,255,200,220});
    float barA = 0.55f+0.45f*sinf(rt*3.f);
    DrawRectangle((int)px,(int)py,(int)(pw*barA),2,{0,255,200,(unsigned char)(barA*175)});

    const char* paused = "// PAUSED //";
    drawNeonText(paused, SW/2-MeasureText(paused,34)/2, (int)(py+18), 34, {0,255,200,255});

    std::string lvlStr = "STAGE " + std::to_string(g.currentStage) + "  --  LEVEL " + std::to_string(g.levelInStage) + " / 8";
    DrawText(lvlStr.c_str(),SW/2-MeasureText(lvlStr.c_str(),14)/2,(int)(py+62),14,{140,200,255,200});

    if (drawBtn(SW/2-90,SH/2-55,180,38,"RESUME",   {5,8,18,220},{0,20,15,240},20))
        { g.screen=Screen::Play; resumeAllBGM(); }
    if (drawBtn(SW/2-90,SH/2-6, 180,38,"SETTINGS", {5,8,18,220},{0,20,15,240},20))
        { g.screenBeforeEdit = Screen::Pause; g.screen=Screen::Settings; pauseAllBGM(); }
    if (drawBtn(SW/2-90,SH/2+43,180,38,"RESTART",  {5,8,18,220},{0,20,15,240},20))
        resetGame();
    if (drawBtn(SW/2-90,SH/2+92,180,36,"MAIN MENU",{5,8,18,220},{0,20,15,240},18))
        { g.screen=Screen::Menu; stopAllBGM(); }

    DrawText("[ ESC to resume ]",SW/2-MeasureText("[ ESC to resume ]",11)/2,(int)(py+ph-16),11,{0,255,200,100});
}

// ── Level complete overlay (full neon scorecard) ─────────────────────────────
void drawLevelComplete() {
    float t  = g.levelCompleteTimer;
    float rt = (float)GetTime();
    DrawRectangle(0,0,SW,SH,{0,0,0,175});
    drawCyberGrid(rt, 14);

    // Panel
    float pw=430.f, ph=270.f, px=SW/2.f-pw/2.f, py=SH/2.f-ph/2.f-10.f;
    DrawRectangle((int)px,(int)py,(int)pw,(int)ph,{4,6,18,240});
    for (int i=4;i>0;i--) DrawRectangleLinesEx({px-i,py-i,pw+i*2,ph+i*2},1.f,{0,255,200,(unsigned char)(14/i)});
    DrawRectangleLinesEx({px,py,pw,ph},2.f,{0,255,200,200});
    drawCornerBrackets(px,py,pw,ph,12.f,{0,255,200,230});

    // Title
    char banner[48]; snprintf(banner,sizeof(banner),"S%d - L%d  COMPLETE!",g.currentStage,g.levelInStage);
    float sc = 1.f + sinf(t*3.f)*0.03f; int bfs=(int)(28*sc);
    drawNeonText(banner, SW/2-MeasureText(banner,bfs)/2, (int)(py+12), bfs, {0,255,200,255});

    // Grade letter (big, colored, pulsing)
    static const char* GRADE_LABELS[] = {"","C","B","A","S"};
    static const Color GRADE_COLORS[] = {{0,0,0,0},{150,150,150,255},{80,220,100,255},{80,180,255,255},{255,215,0,255}};
    int gr = std::max(1, std::min(4, g.lastGrade));
    float gs2 = 1.f + sinf(t*2.5f)*0.07f;
    int gfs = (int)(64*gs2);
    const char* grStr = GRADE_LABELS[gr];
    Color gc = GRADE_COLORS[gr];
    // Glow rings
    for (int i=4;i>0;i--) { Color glow=gc; glow.a=(unsigned char)(20/i);
        DrawText(grStr, SW/2-MeasureText(grStr,gfs)/2-i, (int)(py+46), gfs, glow);
        DrawText(grStr, SW/2-MeasureText(grStr,gfs)/2+i, (int)(py+46), gfs, glow); }
    DrawText(grStr, SW/2-MeasureText(grStr,gfs)/2, (int)(py+46), gfs, gc);

    // Stats rows (fade in)
    float fi = std::min(1.f, t / 0.6f);
    unsigned char fa = (unsigned char)(fi * 255);
    if (fa > 0) {
        float rowY = py + 126.f;
        auto drawRow = [&](const char* label, const char* value, Color vc, float y) {
            DrawText(label, (int)(px+24), (int)y, 14, {140,200,220,(unsigned char)(fa*0.75f)});
            int vw = MeasureText(value, 16);
            DrawText(value, (int)(px+pw-24-vw), (int)(y-1), 16, {vc.r,vc.g,vc.b,fa});
        };
        char tBuf[24]; int sec=(int)g.lastLevelTime;
        snprintf(tBuf,sizeof(tBuf),"%d:%02d",sec/60,sec%60);
        char dBuf[16]; snprintf(dBuf,sizeof(dBuf),"%d",g.lastDeaths);
        char cBuf[32]; snprintf(cBuf,sizeof(cBuf),"%d / %d  (%d%%)",
            g.lastCoinsGot, g.lastCoinsTotal, g.lastCoinsTotal>0?(int)(g.lastCoinPct*100):100);
        char mBuf[16]; snprintf(mBuf,sizeof(mBuf),"x%d",g.lastMaxCombo);

        DrawLineEx({px+18,rowY-4},{px+pw-18,rowY-4},0.8f,{0,255,200,(unsigned char)(fa*0.25f)});
        drawRow("TIME",    tBuf, {180,255,200,255}, rowY);       rowY += 26.f;
        DrawLineEx({px+18,rowY-4},{px+pw-18,rowY-4},0.8f,{0,255,200,(unsigned char)(fa*0.15f)});
        drawRow("DEATHS",  dBuf, g.lastDeaths==0?Color{80,255,120,255}:Color{255,120,80,255}, rowY); rowY += 26.f;
        DrawLineEx({px+18,rowY-4},{px+pw-18,rowY-4},0.8f,{0,255,200,(unsigned char)(fa*0.15f)});
        drawRow("COINS",   cBuf, {241,196,15,255}, rowY);        rowY += 26.f;
        DrawLineEx({px+18,rowY-4},{px+pw-18,rowY-4},0.8f,{0,255,200,(unsigned char)(fa*0.15f)});
        drawRow("COMBO",   mBuf, {255,150,50,255}, rowY);
    }

    // Auto-advance countdown bar
    const float DELAY = 3.5f; float prog = std::min(t/DELAY, 1.f);
    float bW=pw-40.f, bX=px+20.f, bY=py+ph-24.f;
    DrawRectangle((int)bX,(int)bY,(int)bW,10,{10,14,28,220});
    DrawRectangleLinesEx({bX,bY,bW,10},1.f,{0,255,200,55});
    if (prog>0) { DrawRectangle((int)bX,(int)bY,(int)(bW*prog),10,{0,200,160,220});
                  DrawRectangle((int)(bX+bW*prog-2),(int)bY,3,10,{0,255,200,255}); }
    const char* nxtMsg = (g.levelInStage < 10) ? "// NEXT LEVEL..." : "// STAGE COMPLETE!";
    DrawText(nxtMsg, SW/2-MeasureText(nxtMsg,11)/2, (int)(bY+13), 11, {0,255,200,(unsigned char)(prog*180+40)});

    drawScanlines(10);
}

// ── Stage map (horizontal chain of 10 level nodes) ────────────────────────────
void drawStageMap() {
    float t = g.stageMapTimer, rt = (float)GetTime();

    DrawRectangleGradientV(0,0,SW,SH,{4,5,16,255},{6,3,20,255});
    drawCyberGrid(rt, 20);
    for (int i=0;i<22;i++) {
        float px2=fmodf(i*283.f,(float)SW), py2=fmodf(SH-(i*167.f+rt*(9.f+(i%6)*3.f)),(float)SH+10);
        float br=0.5f+0.5f*sinf(rt*2.f+i*.6f);
        DrawCircle((int)px2,(int)py2,1,{0,255,200,(unsigned char)(br*95)});
    }

    // Stage banner panel
    const char* sname = stageName(g.currentStage);
    char banner[32]; snprintf(banner,sizeof(banner),"STAGE %d",g.currentStage);
    int bfs=46, bw=MeasureText(banner,bfs);
    DrawRectangle(SW/2-bw/2-22,14,bw+44,58,{5,8,20,185});
    DrawRectangleLinesEx({(float)(SW/2-bw/2-22),14,(float)(bw+44),58},1.5f,{0,255,200,100});
    drawCornerBrackets(SW/2-bw/2-22.f,14,bw+44.f,58,8.f,{0,255,200,180});
    drawNeonText(banner,SW/2-bw/2,20,bfs,{200,255,240,255});
    DrawText(sname,SW/2-MeasureText(sname,15)/2,70,15,{0,255,200,195});

    // Node chain
    const int N=8; const float nodeR=16.f, spacing=62.f;
    const float startX=SW/2.f-(N-1)*spacing/2.f, rowY=SH/2.f+8.f;
    int cleared=g.stageProgress[g.currentStage], current=g.levelInStage;

    for (int i=0;i<N-1;i++) {
        float ax=startX+i*spacing+nodeR, bx=startX+(i+1)*spacing-nodeR;
        bool done = (i+1)<current || ((i+1)==current && cleared>=current);
        DrawLineEx({ax,rowY},{bx,rowY},3.f,done?Color{0,255,200,155}:Color{35,55,65,120});
        if (done) {
            float pulse=fmodf(rt*1.2f+i*0.32f,1.f);
            float epx=ax+(bx-ax)*pulse;
            DrawCircle((int)epx,(int)rowY,4,{0,255,200,200});
            DrawCircle((int)epx,(int)rowY,2,{200,255,240,255});
        }
    }
    for (int i=0;i<N;i++) {
        float nx=startX+i*spacing; int lv=i+1;
        bool isDone    = lv<current || (lv==current && cleared>=current);
        bool isCurrent = (lv==current);
        float pulse2   = isCurrent ? (1.f+sinf(rt*4.f)*0.14f) : 1.f;
        float r        = nodeR*pulse2;
        if (isCurrent) {
            for (int gi=5;gi>0;gi--)
                DrawCircle((int)nx,(int)rowY,(int)(r+gi*2.5f),{0,255,200,(unsigned char)(gi*14)});
        }
        Color fillC = isDone?Color{0,28,18,245}:isCurrent?Color{0,38,22,245}:Color{8,10,22,205};
        Color ringC = isDone?Color{0,255,200,245}:isCurrent?Color{0,255,200,255}:Color{40,55,75,155};
        DrawCircle((int)nx,(int)rowY,(int)(r+3),ringC);
        DrawCircle((int)nx,(int)rowY,(int)r,fillC);
        char num[4];
        if (isDone){num[0]='\xe2';num[1]='\x9c';num[2]='\x93';num[3]=0;}
        else snprintf(num,sizeof(num),"%d",lv);
        int nfs=isDone?12:14, nw2=MeasureText(num,nfs);
        Color nc=isDone?Color{0,255,200,255}:isCurrent?Color{255,255,255,255}:Color{75,95,115,175};
        if (isCurrent) drawNeonText(num,(int)(nx-nw2/2),(int)(rowY-nfs/2),nfs,nc);
        else           DrawText(num,(int)(nx-nw2/2),(int)(rowY-nfs/2),nfs,nc);
        if (isCurrent) {
            char lbl2[8]; snprintf(lbl2,sizeof(lbl2),"LVL %d",lv);
            int llw=MeasureText(lbl2,10);
            DrawText(lbl2,(int)(nx-llw/2),(int)(rowY+r+8),10,{0,255,200,190});
        }
        if (lv==8) DrawText("BOSS",(int)(nx-MeasureText("BOSS",9)/2),(int)(rowY-r-14),9,{231,76,60,195});
        // Grade badge on completed nodes
        {
            int grd = g.levelGrades[g.currentStage][lv];
            if (grd > 0 && isDone) {
                static const char* GL[] = {"","C","B","A","S"};
                static const Color GC[] = {{},{150,150,150,220},{80,220,100,220},{80,180,255,220},{255,215,0,220}};
                int glw=MeasureText(GL[grd],9);
                DrawRectangle((int)(nx+nodeR-8),(int)(rowY-nodeR-1),glw+6,12,{4,6,18,220});
                DrawRectangleLinesEx({nx+nodeR-8,rowY-nodeR-1,(float)(glw+6),12},0.8f,GC[grd]);
                DrawText(GL[grd],(int)(nx+nodeR-5),(int)(rowY-nodeR+1),9,GC[grd]);
            }
        }
    }

    // "Level incoming" label
    char nextLbl[64]; snprintf(nextLbl,sizeof(nextLbl),"LEVEL %d  --  %s",current,sname);
    int nlw=MeasureText(nextLbl,14);
    DrawRectangle(SW/2-nlw/2-14,(int)(rowY+46),nlw+28,22,{5,8,20,190});
    DrawRectangleLinesEx({(float)(SW/2-nlw/2-14),(float)(rowY+46),(float)(nlw+28),22},1.f,{0,255,200,55});
    DrawText(nextLbl,SW/2-nlw/2,(int)(rowY+50),14,{0,255,200,215});

    // Auto-advance bar
    const float DELAY=2.5f; float prog=std::min(t/DELAY,1.f);
    float barW=244.f, barX=SW/2.f-barW/2.f, barY=SH-52.f;
    DrawRectangle((int)barX,(int)barY,(int)barW,10,{10,14,28,220});
    DrawRectangleLinesEx({barX,barY,barW,10},1.f,{0,255,200,50});
    if (prog>0) { DrawRectangle((int)barX,(int)barY,(int)(barW*prog),10,{0,200,160,220});
                  DrawRectangle((int)(barX+barW*prog-2),(int)barY,3,10,{0,255,200,255}); }
    const char* stMsg = prog<0.92f?"// LOADING LEVEL...":"// STARTING...";
    DrawText(stMsg,SW/2-MeasureText(stMsg,11)/2,(int)(barY+14),11,{0,255,200,145});

    drawScanlines(12);
    drawCloseBtn();
}

// ── Stage complete celebration ─────────────────────────────────────────────────
void drawStageComplete() {
    float t = g.stageCompleteTimer, rt = (float)GetTime();

    DrawRectangle(0,0,SW,SH,{0,0,0,225});
    drawCyberGrid(rt,12);

    // Enhanced burst fireworks
    for (int i=0;i<14;i++) {
        float phase=fmodf(t*0.65f+i*0.44f,3.8f);
        if (phase>2.2f) continue;
        float bx=35.f+(float)(i%7)*116.f, by=45.f+(float)(i/7)*145.f;
        float rad=phase*165.f, fade=1.f-phase/2.2f;
        for (int j=0;j<28;j++) {
            float ang=(float)j/28.f*6.28318f;
            Color fw=ColorFromHSV(fmodf(i*26.f+j*13.f+rt*70.f,360.f),.95f,1.f);
            fw.a=(unsigned char)(fade*220.f);
            DrawCircle((int)(bx+cosf(ang)*rad),(int)(by+sinf(ang)*rad),3,fw);
        }
        if (phase<0.9f) for (int j=0;j<8;j++) {
            float ang=(float)j/8.f*6.28318f+phase;
            DrawCircle((int)(bx+cosf(ang)*phase*55),(int)(by+sinf(ang)*phase*55),2,{255,255,255,(unsigned char)(fade*175)});
        }
    }

    // Central panel
    float pw=440.f, ph=215.f, px=SW/2.f-pw/2.f, py=SH/2.f-ph/2.f-15.f;
    DrawRectangle((int)px,(int)py,(int)pw,(int)ph,{4,6,18,232});
    Color bHue=ColorFromHSV(fmodf(rt*55.f,360.f),.85f,1.f);
    for (int i=4;i>0;i--) { Color gc=bHue; gc.a=(unsigned char)(18/i);
        DrawRectangleLinesEx({px-i,py-i,pw+i*2,ph+i*2},1.f,gc); }
    DrawRectangleLinesEx({px,py,pw,ph},2.f,bHue);
    drawCornerBrackets(px,py,pw,ph,14.f,bHue);
    float topP=std::min(t/0.5f,1.f);
    DrawRectangle((int)px,(int)py,(int)(pw*topP),2,{(unsigned char)bHue.r,(unsigned char)bHue.g,(unsigned char)bHue.b,(unsigned char)(170)});

    char banner[32]; snprintf(banner,sizeof(banner),"STAGE %d",g.currentStage);
    float sc=1.f+sinf(t*3.f)*0.04f; int bfs=(int)(52*sc);
    Color bannerC=ColorFromHSV(fmodf(rt*65.f,360.f),0.85f,1.f);
    int bannerW=MeasureText(banner,bfs);
    for (int i=3;i>0;i--) { Color gc=bannerC; gc.a=(unsigned char)(22/i);
        DrawText(banner,(int)(SW/2-bannerW*sc/2)-i,(int)(py+16),bfs,gc);
        DrawText(banner,(int)(SW/2-bannerW*sc/2)+i,(int)(py+16),bfs,gc); }
    DrawText(banner,(int)(SW/2-bannerW*sc/2),(int)(py+16),bfs,bannerC);

    const char* sub="COMPLETE!"; int sfs=(int)(30*sc);
    drawNeonText(sub,SW/2-MeasureText(sub,sfs)/2,(int)(py+74),sfs,{0,255,200,255});

    const char* sname=stageName(g.currentStage);
    DrawText(sname,SW/2-MeasureText(sname,16)/2,(int)(py+112),16,{180,240,220,195});

    std::string sc2="SCORE: "+std::to_string(g.score);
    DrawText(sc2.c_str(),SW/2-MeasureText(sc2.c_str(),15)/2,(int)(py+138),15,{255,255,255,205});

    if (g.currentStage<10) {
        char nxt[48]; snprintf(nxt,sizeof(nxt),">> NEXT: STAGE %d  -  %s <<",g.currentStage+1,stageName(g.currentStage+1));
        float a=std::min(1.f,std::max(0.f,(t-1.4f)/0.6f));
        drawCornerBrackets(px+10,py+160,pw-20,26,5.f,{0,255,200,(unsigned char)(a*140)});
        DrawText(nxt,SW/2-MeasureText(nxt,13)/2,(int)(py+168),13,{0,255,200,(unsigned char)(a*215)});
    }

    // Countdown bar
    const float DELAY=3.5f; float prog=std::min(t/DELAY,1.f);
    float barW=224.f, barX=SW/2.f-barW/2.f, barY=SH-50.f;
    DrawRectangle((int)barX,(int)barY,(int)barW,10,{10,14,28,220});
    DrawRectangleLinesEx({barX,barY,barW,10},1.f,{0,255,200,55});
    Color barFill=ColorFromHSV(fmodf(rt*55.f,360.f),.8f,1.f);
    if (prog>0) DrawRectangle((int)barX,(int)barY,(int)(barW*prog),10,barFill);

    drawScanlines(10);
    drawCloseBtn();
}

// ── Grand winner screen ────────────────────────────────────────────────────────
void drawGameWinner() {
    float t = g.gameWinTimer, rt = (float)GetTime();

    DrawRectangleGradientV(0,0,SW,SH,{3,1,12,255},{8,3,25,255});
    drawCyberGrid(rt,14);

    // Dense starfield
    for (int i=0;i<100;i++) {
        float sx2=fmodf((float)(i*241+67),(float)SW), sy2=fmodf((float)(i*137+53),(float)SH);
        float br=0.5f+0.5f*sinf(rt*1.4f+i*.65f);
        int ri=i%5==0?3:i%3==0?2:1;
        DrawCircle((int)sx2,(int)sy2,ri,{255,255,255,(unsigned char)(br*200)});
        if (i%8==0) {
            Color sp={255,255,220,(unsigned char)(br*75)};
            DrawLineEx({sx2-3,sy2},{sx2+3,sy2},0.7f,sp);
            DrawLineEx({sx2,sy2-3},{sx2,sy2+3},0.7f,sp);
        }
    }

    // Continuous fireworks (dense)
    for (int i=0;i<18;i++) {
        float phase=fmodf(t*0.5f+i*0.37f,3.2f);
        if (phase>2.2f) continue;
        float bx=28.f+(float)(i%9)*90.f, by=22.f+(float)(i/9)*158.f;
        float rad=phase*205.f, fade=1.f-phase/2.2f;
        for (int j=0;j<36;j++) {
            float ang=(float)j/36.f*6.28318f;
            Color fw=ColorFromHSV(fmodf(i*20.f+j*10.f+rt*90.f,360.f),.95f,1.f);
            fw.a=(unsigned char)(fade*240.f);
            DrawCircle((int)(bx+cosf(ang)*rad),(int)(by+sinf(ang)*rad),3,fw);
            if (j%6==0) DrawCircle((int)(bx+cosf(ang)*rad*.45f),(int)(by+sinf(ang)*rad*.45f),2,fw);
        }
    }

    // Glowing orb / trophy
    float tr2=3.f+sinf(rt*2.f)*2.f, orb=32.f+tr2;
    DrawCircle(SW/2,(int)(SH/2-82),(int)(orb+18),{255,215,0,(unsigned char)(28+sinf(rt*2.f)*18)});
    DrawCircle(SW/2,(int)(SH/2-82),(int)(orb+10),{255,215,0,(unsigned char)(55+sinf(rt*2.5f)*28)});
    DrawCircle(SW/2,(int)(SH/2-82),(int)orb,     {255,200,0,232});
    DrawCircle(SW/2,(int)(SH/2-82),(int)(orb-9), {255,240,100,205});
    const char* star="\xe2\x98\x85";
    drawNeonText(star,SW/2-MeasureText(star,28)/2,(int)(SH/2-98),28,{255,255,200,255});

    // Letter-by-letter "YOU ARE THE CHAMPION!"
    const char* line1="YOU ARE THE"; const char* line2="CHAMPION!";
    float scl=1.f+sinf(rt*1.6f)*0.03f;
    int fs1=(int)(38*scl), fs2=(int)(58*scl);
    char lbuf2[2]={0,0};

    int ox1=SW/2-MeasureText(line1,fs1)/2;
    for (int i=0;i<(int)strlen(line1);i++) {
        float prog2=std::max(0.f,std::min(1.f,(t-0.04f*i)/0.25f));
        float dy=(1.f-prog2)*28.f; unsigned char aa=(unsigned char)(prog2*255);
        lbuf2[0]=line1[i]; int lw=MeasureText(lbuf2,fs1);
        Color gc={0,255,200,aa}; gc.a=(unsigned char)(aa*0.28f);
        DrawText(lbuf2,ox1-1,(int)(SH/2-140+dy),fs1,gc);
        DrawText(lbuf2,ox1+1,(int)(SH/2-140+dy),fs1,gc);
        DrawText(lbuf2,ox1,(int)(SH/2-140+dy),fs1,{220,240,255,aa});
        ox1+=lw+1;
    }
    int ox2=SW/2-MeasureText(line2,fs2)/2;
    for (int i=0;i<(int)strlen(line2);i++) {
        float prog2=std::max(0.f,std::min(1.f,(t-0.35f-0.055f*i)/0.3f));
        float dy=(1.f-prog2)*38.f; unsigned char aa=(unsigned char)(prog2*255);
        lbuf2[0]=line2[i]; int lw=MeasureText(lbuf2,fs2);
        Color cc=ColorFromHSV(fmodf(i*40.f+rt*55.f,360.f),.9f,1.f); cc.a=aa;
        Color gc=cc; gc.a=(unsigned char)(aa*0.24f);
        DrawText(lbuf2,ox2-2,(int)(SH/2-86+dy),fs2,gc);
        DrawText(lbuf2,ox2+2,(int)(SH/2-86+dy),fs2,gc);
        DrawText(lbuf2,ox2,(int)(SH/2-86+dy),fs2,cc);
        ox2+=lw+1;
    }

    // Stats panel (fade in after 2s)
    float statsA=std::max(0.f,std::min(1.f,(t-2.f)/0.8f));
    unsigned char sa=(unsigned char)(statsA*255);
    if (sa>0) {
        float spw=340.f, sph=96.f, spx=SW/2.f-spw/2.f, spy=SH/2.f+10.f;
        DrawRectangle((int)spx,(int)spy,(int)spw,(int)sph,{5,8,20,(unsigned char)(sa*0.9f)});
        for (int i=3;i>0;i--) DrawRectangleLinesEx({spx-i,spy-i,spw+i*2,sph+i*2},1.f,{0,255,200,(unsigned char)(sa/10/i)});
        DrawRectangleLinesEx({spx,spy,spw,sph},1.5f,{0,255,200,(unsigned char)(sa*0.7f)});
        drawCornerBrackets(spx,spy,spw,sph,8.f,{0,255,200,sa});
        std::string fs3="FINAL SCORE: "+std::to_string(g.score);
        DrawText(fs3.c_str(),SW/2-MeasureText(fs3.c_str(),18)/2,(int)(spy+8),18,{255,255,255,sa});
        int sec=(int)g.elapsed; char ts2[32]; snprintf(ts2,sizeof(ts2),"TOTAL TIME: %d:%02d",sec/60,sec%60);
        DrawText(ts2,SW/2-MeasureText(ts2,14)/2,(int)(spy+30),14,{200,255,200,sa});
        char dts[32]; snprintf(dts,sizeof(dts),"TOTAL DEATHS: %d",g.totalDeaths);
        Color dtc = g.totalDeaths==0?Color{255,215,0,sa}:Color{255,120,80,sa};
        DrawText(dts,SW/2-MeasureText(dts,14)/2,(int)(spy+50),14,dtc);
        if (g.highScore>0) { std::string hs="HIGH SCORE: "+std::to_string(g.highScore);
            DrawText(hs.c_str(),SW/2-MeasureText(hs.c_str(),13)/2,(int)(spy+72),13,{255,215,0,sa}); }
    }

    // Buttons (appear after 3s)
    if (t>3.f) {
        if (drawBtn(SW/2-132,SH/2+100,122,46,"PLAY AGAIN",{5,8,18,220},{0,20,15,240},18)) resetGame();
        if (drawBtn(SW/2+14, SH/2+100,118,46,"MENU",      {5,8,18,220},{0,20,15,240},18))
            { g.screen=Screen::Menu; stopAllBGM(); }
        float ra=0.5f+0.5f*sinf(rt*1.5f);
        DrawText("[ or press R ]",SW/2-MeasureText("[ or press R ]",12)/2,SH/2+154,12,{0,255,200,(unsigned char)(ra*145)});
    }

    drawScanlines(10);
    drawCloseBtn();
}

// ── Game scene ────────────────────────────────────────────────────────────────
void drawScene() {
    float t = (float)GetTime();
    float decay = g.shakeDecay > 0 ? g.shakeTimer / g.shakeDecay : 0.f;
    float sx = g.shakeTimer > 0 ? sinf(t*38.f) * g.shakeAmount * decay : 0.f;
    float sy = g.shakeTimer > 0 ? cosf(t*31.f) * g.shakeAmount * decay * .6f : 0.f;

    DrawRectangleGradientV(0,0,SW,SH,skyTop(),skyBot());
    drawStars(g.cameraX);
    drawHorizonSun(g.cameraX);
    drawPerspectiveGrid(g.cameraX);
    drawTrees(g.cameraX); drawClouds(g.cameraX);
    int _th = stageTheme(g.currentStage);
    if (_th==2) drawRain();
    if (_th==4) drawLava();
    if (_th==5) drawSnow();

    Camera2D cam = {}; cam.target = {g.cameraX+sx, sy}; cam.zoom = 1.f;
    BeginMode2D(cam);

    // Rising lava floor (stage 9)
    if (g.risingLavaY < 9999.f) {
        float lavaT = (float)GetTime();
        float lavaTop = g.risingLavaY;
        // Lava body
        DrawRectangle((int)g.cameraX, (int)lavaTop, SW, (int)(SH * 2), {180,50,10,255});
        // Animated surface
        for (int i = 0; i < 30; i++) {
            float lx = g.cameraX + fmodf(i * 87.3f + lavaT * 18.f, (float)SW);
            float ly = lavaTop + sinf(lavaT * 4.f + i * 1.1f) * 5.f;
            DrawCircle((int)lx, (int)ly, 7 + (i % 3) * 3, {240,130,20,200});
        }
        // Glow at top
        for (int i = 0; i < 12; i++)
            DrawRectangle((int)g.cameraX, (int)(lavaTop - i * 2), SW, 3,
                {255,80,20,(unsigned char)(50 - i * 4)});
    }

    for (const auto& p : g.platforms) drawPlatform(p);

    for (const auto& c : g.coins) {
        if (c.collected) continue;
        float bob  = sinf(t*2.2f + c.x*.09f) * 3.f;
        float spin = fmodf(t*3.2f + c.x*.055f, PI*2.f);
        int cw = (int)(fabsf(cosf(spin))*8.f) + 1;
        float cgl = 0.5f + 0.5f * sinf(t*3.f + c.x*.08f);
        drawBloom(c.x, c.y+bob, 8.f, {255,190,11,(unsigned char)(cgl*45+18)});
        DrawEllipse((int)c.x,(int)(c.y+bob),cw+2,9,{200,150,8,255});
        DrawEllipse((int)c.x,(int)(c.y+bob),cw,8,{255,190,11,255});
        if (cosf(spin) > 0.4f) {
            DrawEllipse((int)c.x-2,(int)(c.y+bob-2),(int)(cosf(spin)*5.f),4,{255,245,150,100});
        }
    }

    for (const auto& m : g.mushrooms) if (!m.collected) drawMushroom(m);
    for (const auto& cp : g.checkpoints) drawCheckpoint(cp);
    for (const auto& e : g.enemies) {
        if (e.alive) drawEnemy(e);
        else if (e.deathTimer > 0.f) drawDeathAnim(e);
    }

    for (const auto& p : g.projectiles) if (p.active) {
        float ang = t*8.f * (p.vx>0?1.f:-1.f);
        DrawCircle((int)p.x,(int)p.y,6,{231,76,60,220});
        DrawCircle((int)p.x,(int)p.y,4,{255,200,80,255});
        for (int i = 0; i < 3; i++) {
            float ta = ang + i*2.1f;
            DrawCircle((int)(p.x+cosf(ta)*5.f),(int)(p.y+sinf(ta)*5.f),2,{231,76,60,(unsigned char)(120-i*35)});
        }
    }

    for (const auto& s : g.spikes)  drawSpike(s);
    for (const auto& geo : g.geysers) drawGeyser(geo);
    drawFlag();
    for (const auto& pt : g.particles) DrawCircle((int)pt.x,(int)pt.y,(int)pt.size,pt.col);

    for (const auto& pp : g.popups) {
        float a    = pp.life / pp.maxLife;
        float rise = (1.f - a) * 18.f;
        int fs2 = 13 + (pp.text[0]=='+' ? 0 : 2);
        DrawText(pp.text, (int)pp.x-MeasureText(pp.text,fs2)/2, (int)(pp.y-rise), fs2,
            Color{pp.col.r,pp.col.g,pp.col.b,(unsigned char)(a*255)});
    }

    // Player with squash/stretch and dash afterimage
    float sqsh_off = 0.f;
    if (g.player.squashTimer > 0.f) {
        float t2 = 1.f - g.player.squashTimer / 0.18f;
        sqsh_off = sinf(t2 * PI) * (-4.f);
    }
    float breathY = (g.player.vx==0 && g.player.grounded) ? sinf(t*2.f)*.8f : 0.f;
    // Speed lines — appear at 70% MOVE_SPEED, full length at max dash speed
    {
        float spd = fabsf(g.player.vx);
        float threshold = MOVE_SPEED * SPEEDLINE_THRESH;
        if (spd > threshold) {
            float t = std::min((spd - threshold) / (MOVE_SPEED * DASH_SPEED_MULT - threshold), 1.f);
            float lineLen  = t * 55.f;
            float startX   = g.player.facing > 0 ? g.player.x : g.player.x + g.player.w;
            unsigned char la = (unsigned char)(t * 140 + 20);
            for (int i = 0; i < 5; i++) {
                int ly = (int)(g.player.y + 6.f + i * 6.f);
                DrawLine((int)startX, ly,
                         (int)(startX - g.player.facing * lineLen), ly,
                         {200, 230, 255, la});
            }
        }
    }
    for (int i = g.player.aiCount - 1; i >= 0; i--) {
        float t = (float)(g.player.aiCount - i) / (float)AFTERIMAGE_COUNT;
        unsigned char alpha = (unsigned char)(t * 110);
        DrawRectangle((int)g.player.aiX[i], (int)g.player.aiY[i],
                      (int)g.player.w, (int)g.player.h,
                      {0, 200, 255, alpha});
    }
    drawPlayerAt(g.player.x, g.player.y+breathY+sqsh_off,
                 g.player.facing, g.player.walkFrame,
                 g.player.grounded, g.player.powered, g.player.invFrames);

    EndMode2D();

    // Chain vignette pulse (magenta at 6-9, rainbow at 10+)
    if (g.vignettePulse > 0.f) {
        Color vc = g.combo >= 10
            ? ColorFromHSV(fmodf(g.menuTimer * 120.f, 360.f), 1.f, 1.f)
            : Color{255, 0, 180, 255};
        vc.a = (unsigned char)(g.vignettePulse * 160);
        DrawRectangle(0,      0,      SW,  70,   vc);
        DrawRectangle(0,      SH-70,  SW,  70,   vc);
        DrawRectangle(0,      0,      70,  SH,   vc);
        DrawRectangle(SW-70,  0,      70,  SH,   vc);
    }

    // Stage 8: reduced visibility darkness with small light around player
    if (g.currentStage == 8) {
        float px2 = g.player.x - g.cameraX + g.player.w / 2.f;
        float py2 = g.player.y + g.player.h / 2.f;
        // Dark overlay with circular cutout (fake radial gradient via concentric rects)
        DrawRectangle(0,0,SW,SH,{0,0,8,195});
        const float radius = 130.f;
        for (int i = (int)radius; i >= 0; i -= 4) {
            float alpha = (1.f - (float)i / radius);
            DrawCircle((int)px2,(int)py2, i, {0,0,8,(unsigned char)(alpha * 195)});
        }
        // Clear the circle around player (draw a bright spot)
        DrawCircle((int)px2,(int)py2,(int)radius, {0,0,0,0});
    }

    // Wind indicator in HUD (stages 5, 6, 9)
    if (g.windForce != 0.f) {
        float t2 = (float)GetTime();
        float wf = 0.5f + 0.5f * sinf(t2 * 3.f);
        const char* windLbl = g.windForce < 0 ? "< WIND" : "WIND >";
        int wlw = MeasureText(windLbl, 11);
        DrawRectangle(4, 104, wlw + 14, 18, {5,8,20,190});
        DrawRectangleLinesEx({4,104,(float)(wlw+14),18},0.8f,{100,180,255,(unsigned char)(wf*120+40)});
        DrawText(windLbl, 10, 107, 11, {120,190,255,(unsigned char)(wf*180+60)});
    }

    drawVignette();

    // Danger vignette on last life
    if (g.lives == 1) {
        float pulse = 0.55f + 0.45f * sinf(t*3.8f);
        for (int i = 0; i < 22; i++)
            DrawRectangleLinesEx({(float)i,(float)i,(float)(SW-i*2),(float)(SH-i*2)},
                2.f, {200,0,0,(unsigned char)((22-i)*5*pulse)});
    }

    // Chromatic aberration on damage flash
    if (g.flashTimer > 0.f) {
        float str = std::min(g.flashTimer / 0.4f, 1.f);
        int off = (int)(str * 7);
        DrawRectangleGradientH(0,0,off*4,SH,{255,30,30,(unsigned char)(str*55)},{0,0,0,0});
        DrawRectangleGradientH(SW-off*4,0,off*4,SH,{0,0,0,0},{30,30,255,(unsigned char)(str*55)});
    }

    // ── CYBERPUNK HUD ─────────────────────────────────────────────────────────

    // Score panel (top-left)
    {
        std::string scStr = std::to_string(g.score);
        int sw2 = MeasureText(scStr.c_str(),22);
        DrawRectangle(4,4,sw2+54,30,{5,8,20,215});
        DrawRectangleLinesEx({4,4,(float)(sw2+54),30},1.f,{0,255,200,80});
        DrawText("SC:",8,10,12,{0,255,200,150});
        drawNeonText(scStr.c_str(),32,8,22,{255,255,255,255});
    }
    // High score (top-right)
    if (g.highScore > 0) {
        std::string hsStr = std::to_string(g.highScore);
        int hw = MeasureText(hsStr.c_str(),16);
        DrawRectangle(SW-hw-52,4,hw+48,26,{5,8,20,200});
        DrawRectangleLinesEx({(float)(SW-hw-52),4,(float)(hw+48),26},1.f,{255,215,0,65});
        DrawText("BEST:",SW-hw-46,8,11,{255,215,0,130});
        DrawText(hsStr.c_str(),SW-hw-8,7,16,{255,215,0,230});
    }
    // Timer (top-center)
    {
        int s2 = (int)g.elapsed;
        char ts[16]; snprintf(ts,sizeof(ts),"%d:%02d",s2/60,s2%60);
        int tw2 = MeasureText(ts,20);
        DrawRectangle(SW/2-tw2/2-18,4,tw2+36,26,{5,8,20,200});
        DrawRectangleLinesEx({(float)(SW/2-tw2/2-18),4,(float)(tw2+36),26},1.f,{0,255,200,65});
        DrawText(ts,SW/2-tw2/2,7,20,{200,255,220,240});
    }
    // Lives (segmented neon bar)
    {
        int liveCount = std::max(g.lives,0);
        DrawRectangle(4,38,72,18,{5,8,20,200});
        DrawRectangleLinesEx({4,38,72,18},1.f,{0,255,200,55});
        for (int i=0;i<3;i++) {
            float sx2=8.f+i*22.f; bool alive=(i<liveCount);
            DrawRectangle((int)sx2,41,18,12,alive?Color{200,50,40,230}:Color{35,10,10,160});
            DrawRectangleLinesEx({sx2,41,18,12},0.8f,alive?Color{255,120,110,100}:Color{50,15,15,70});
            if (alive) drawHeart(sx2+2.f,42.f,{255,140,130,255});
        }
    }
    // Dash cooldown bar
    {
        bool dashReady = (g.player.dashCooldown<=0.f);
        float fill = dashReady ? 1.f : std::max(0.f,1.f-g.player.dashCooldown);
        Color dc = dashReady?Color{0,255,200,220}:Color{0,130,110,180};
        DrawRectangle(4,60,62,8,{10,12,25,200});
        DrawRectangleLinesEx({4,60,62,8},0.8f,{0,255,200,45});
        if (fill>0) { DrawRectangle(4,60,(int)(62*fill),8,dc);
                      if (dashReady) DrawRectangle(4,60,62,2,{0,255,200,110}); }
        DrawText(dashReady?"DASH":"----",4,70,9,dashReady?Color{0,255,200,200}:Color{50,90,90,160});
    }
    // Stage/level indicator
    {
        char lvlBuf[24]; snprintf(lvlBuf,sizeof(lvlBuf),"S%d-L%d/8",g.currentStage,g.levelInStage);
        int lw2=MeasureText(lvlBuf,12);
        DrawRectangle(4,82,lw2+12,18,{5,8,20,180});
        DrawRectangleLinesEx({4,82,(float)(lw2+12),18},0.8f,{160,160,255,45});
        DrawText(lvlBuf,9,85,12,{160,180,255,200});
    }
    // Boss health bar (top-centre, visible when boss level active)
    if (g.levelInStage == 8) {
        const Enemy* boss = nullptr;
        for (const auto& en : g.enemies) if (en.type==3 && en.alive) { boss=&en; break; }
        if (boss) {
            int maxH = (g.currentStage <= 3) ? (1 + g.currentStage) : (2 + g.currentStage / 2);
            const char* bossNames[] = {"","WARDEN","SENTINEL","INFERNO"};
            const char* phaseStr = nullptr;
            if      (g.currentStage==1) phaseStr = boss->health==1 ? "PHASE 2" : "PHASE 1";
            else if (g.currentStage==2) phaseStr = boss->health==1 ? "PHASE 3" : boss->health==2 ? "PHASE 2" : "PHASE 1";
            else                        phaseStr = boss->health<=2  ? "ENRAGED"  : "PHASE 1";
            float bw=280.f, bh=13.f, bx=SW/2.f-bw/2.f, by=36.f;
            float pulse = 0.5f+0.5f*sinf(t*5.f);
            Color bc = boss->col;
            DrawRectangle((int)(bx-4),(int)(by-16),(int)(bw+8),(int)(bh+20),{5,8,20,230});
            for (int i=3;i>0;i--) DrawRectangleLinesEx({bx-4-i,by-16-i,bw+8+i*2.f,bh+20+i*2.f},1.f,{bc.r,bc.g,bc.b,(unsigned char)(12/i)});
            DrawRectangleLinesEx({bx-4,by-16,bw+8,bh+20},1.5f,{bc.r,bc.g,bc.b,(unsigned char)(pulse*120+60)});
            const char* bn = g.currentStage<=3 ? bossNames[g.currentStage] : "BOSS";
            int bnw=MeasureText(bn,11);
            drawNeonText(bn,SW/2-bnw/2,(int)(by-14),11,{bc.r,bc.g,bc.b,220});
            float segW = (bw-(maxH-1)*4.f) / maxH;
            for (int i=0;i<maxH;i++) {
                float sx=bx+i*(segW+4.f);
                bool filled=(i<boss->health);
                DrawRectangle((int)sx,(int)by,(int)segW,(int)bh,filled?bc:Color{20,25,35,200});
                if (filled) { DrawRectangle((int)sx,(int)by,(int)segW,3,{255,255,255,45}); }
                DrawRectangleLinesEx({sx,by,segW,bh},1.f,filled?Color{255,255,255,60}:Color{35,45,55,140});
            }
            int pw=MeasureText(phaseStr,9);
            DrawText(phaseStr,(int)(bx+bw-pw-2),(int)(by+bh+2),9,{255,200,100,155});
        }
    }
    // Boss warning intro (first 2.5s of boss level)
    if (g.levelInStage == 8) {
        float age2 = g.elapsed - g.levelStartTime;
        if (age2 < 2.5f) {
            float fi = age2 < 0.3f ? age2/0.3f : (age2 > 2.0f ? (2.5f-age2)/0.5f : 1.f);
            fi = std::max(0.f, std::min(1.f, fi));
            unsigned char ba=(unsigned char)(fi*230);
            const char* bnames[4]={"","THE WARDEN","THE SENTINEL","THE INFERNO"};
            const char* bsubs[4] ={"","Stage Guardian","Elite Hunter","Volcanic Fury"};
            int stg = std::min(g.currentStage,3);
            float wby=SH/2.f-38.f;
            DrawRectangle(0,(int)wby,SW,76,{8,0,0,(unsigned char)(fi*140)});
            DrawRectangleLinesEx({0,wby,(float)SW,76},2.f,{255,50,20,(unsigned char)(fi*.6f*255)});
            float wblk = age2 < 1.6f ? (sinf(age2*22.f) > 0 ? 1.f : 0.f) : 1.f;
            const char* warn="!! WARNING !!";
            int ww=MeasureText(warn,26);
            drawNeonText(warn,SW/2-ww/2,(int)(wby+6),26,{255,50,20,(unsigned char)(wblk*ba)});
            const char* bn2=bnames[stg];
            DrawText(bn2,SW/2-MeasureText(bn2,16)/2,(int)(wby+44),16,{255,200,100,ba});
        }
    }
    if (isIceStage(g.currentStage) && g.player.grounded) {
        float ia = 0.6f+0.4f*sinf(t*3.f);
        DrawText("ICE!",4,103,10,{140,210,255,(unsigned char)(ia*200)});
    }
    // SUPER indicator
    if (g.player.powered) {
        float gp = 0.5f+0.5f*sinf(t*5.f);
        const char* spw2 = ">> SUPER <<";
        int sw3 = MeasureText(spw2,20);
        DrawRectangle(SW/2-sw3/2-10,SH-38,sw3+20,28,{5,8,20,215});
        DrawRectangleLinesEx({(float)(SW/2-sw3/2-10),(float)(SH-38),(float)(sw3+20),28},1.5f,{255,215,0,(unsigned char)(gp*200)});
        drawNeonText(spw2,SW/2-sw3/2,SH-34,20,{255,215,0,255});
    }
    // Combo counter
    if (g.combo > 1) {
        char cb[32]; snprintf(cb,sizeof(cb),"x%d COMBO",g.combo);
        float ca  = std::min(1.f,g.comboTimer*.8f);
        float cy2 = SH/2.f-58.f+sinf(t*5.f)*5.f;
        int cfs   = 24+std::min(g.combo-2,6)*4;
        Color cc  = g.combo >= 10 ? ColorFromHSV(fmodf(t*180.f,360.f),1.f,1.f)
                  : g.combo >= 6  ? Color{255,  0, 180, 255}
                  : g.combo >= 3  ? Color{  0, 200, 255, 255}
                  :                 Color{255, 255, 200, 255};
        cc.a = (unsigned char)(ca*255);
        DrawText(cb,SW/2-MeasureText(cb,cfs)/2+2,(int)cy2+2,cfs,{0,0,0,(unsigned char)(ca*120)});
        for (int i=2;i>0;i--) { Color gc=cc; gc.a=(unsigned char)(ca*38/i);
            DrawText(cb,SW/2-MeasureText(cb,cfs)/2-i,(int)cy2,cfs,gc);
            DrawText(cb,SW/2-MeasureText(cb,cfs)/2+i,(int)cy2,cfs,gc); }
        DrawText(cb,SW/2-MeasureText(cb,cfs)/2,(int)cy2,cfs,cc);
    }
    // Boss warning
    {
        bool anyBoss = false;
        for (const auto& enemy : g.enemies) if (enemy.type==3&&enemy.alive&&enemy.x-g.cameraX<SW+100) anyBoss=true;
        if (anyBoss) {
            float pulse = 0.5f+0.5f*sinf(t*6.f);
            const char* bwarn = "!! BOSS !!";
            int bw2 = MeasureText(bwarn,24);
            DrawRectangle(SW/2-bw2/2-14,SH-62,bw2+28,30,{5,8,20,205});
            DrawRectangleLinesEx({(float)(SW/2-bw2/2-14),(float)(SH-62),(float)(bw2+28),30},1.5f,
                {231,76,60,(unsigned char)(pulse*200)});
            drawNeonText(bwarn,SW/2-bw2/2,SH-56,24,{231,76,60,(unsigned char)(100+pulse*155)});
        }
    }
    // Achievement notification
    if (!g.achievementNotifs.empty()) {
        const auto& an = g.achievementNotifs.front();
        float fa;
        if (an.life>3.f)      fa=(3.5f-an.life)/.5f;
        else if (an.life<.4f) fa=an.life/.4f;
        else                  fa=1.f;
        fa=std::max(0.f,std::min(1.f,fa));
        unsigned char aa=(unsigned char)(fa*240);
        DrawRectangle(4,115,240,46,{5,8,20,(unsigned char)(aa*.9f)});
        for (int i=3;i>0;i--) DrawRectangleLinesEx({4.f-i,115.f-i,240.f+i*2.f,46.f+i*2.f},1.f,{255,215,0,(unsigned char)(aa/8/i)});
        DrawRectangleLinesEx({4,115,240,46},1.5f,{255,215,0,aa});
        drawCornerBrackets(4,115,240,46,6.f,{255,215,0,aa});
        DrawText("ACHIEVEMENT!",10,120,11,{255,215,0,aa});
        DrawText(an.name,10,134,13,{255,255,255,aa});
    }
    // First-run tutorial: symbol key overlay on S1-L1 for first 4s
    if (g.currentStage==1 && g.levelInStage==1 && g.screen==Screen::Play && !(g.tutorialMask & 0x80)) {
        float age  = g.elapsed - g.levelStartTime;
        float fade = age < 0.4f ? age/0.4f : (age > 3.2f ? (4.f-age)/0.8f : 1.f);
        fade = std::max(0.f, std::min(1.f, fade));
        unsigned char oa = (unsigned char)(fade * 220);

        // Helper: draw a keycap at (kx,ky) size kw×kh
        auto keycap = [&](float kx, float ky, float kw, float kh) {
            DrawRectangleRounded({kx+2.f,ky+2.f,kw,kh},0.28f,4,{0,0,10,(unsigned char)(oa*.85f)});
            DrawRectangleRounded({kx,ky,kw,kh},0.28f,4,{28,36,68,(unsigned char)(oa*.95f)});
            DrawRectangleRounded({kx+2.f,ky+2.f,kw-4.f,kh*0.45f},0.3f,4,{80,110,160,(unsigned char)(oa*.4f)});
            DrawRectangleLinesEx({kx,ky,kw,kh},1.5f,{0,220,255,(unsigned char)(oa*.8f)});
        };

        float cy2 = SH - 96.f;
        float grpGap = 96.f;
        float cx1 = SW/2.f - grpGap - 24.f; // LEFT group center
        float cx2 = SW/2.f;                  // JUMP group center
        float cx3 = SW/2.f + grpGap + 24.f; // DASH group center

        // Left / Right arrow keys
        float akW=26.f, akH=24.f;
        keycap(cx1-akW-3.f, cy2, akW, akH);
        DrawTriangle({cx1-akW+2.f, cy2+akH/2.f}, {cx1-5.f, cy2+5.f}, {cx1-5.f, cy2+akH-5.f}, {0,255,200,(unsigned char)(oa*.9f)});
        keycap(cx1+3.f, cy2, akW, akH);
        DrawTriangle({cx1+akW+1.f, cy2+akH/2.f}, {cx1+8.f, cy2+5.f}, {cx1+8.f, cy2+akH-5.f}, {0,255,200,(unsigned char)(oa*.9f)});
        DrawText("MOVE", (int)(cx1 - MeasureText("MOVE",10)/2.f), (int)(cy2+akH+5.f), 10, {0,220,200,(unsigned char)(oa*.8f)});

        // Spacebar
        float spW=58.f, spH=22.f;
        keycap(cx2-spW/2.f, cy2+1.f, spW, spH);
        DrawText("JUMP", (int)(cx2 - MeasureText("JUMP",10)/2.f), (int)(cy2+spH+8.f), 10, {0,220,200,(unsigned char)(oa*.8f)});

        // Shift key
        float shW=42.f, shH=24.f;
        keycap(cx3-shW/2.f, cy2, shW, shH);
        // Up-arrow inside shift key
        float sax=cx3, say=cy2+shH/2.f;
        DrawTriangle({sax,say-7.f},{sax-6.f,say+1.f},{sax+6.f,say+1.f},{0,255,200,(unsigned char)(oa*.9f)});
        fillRect(sax-3.f,say+1.f,6.f,5.f,{0,255,200,(unsigned char)(oa*.9f)});
        DrawText("DASH", (int)(cx3 - MeasureText("DASH",10)/2.f), (int)(cy2+shH+5.f), 10, {0,220,200,(unsigned char)(oa*.8f)});

        // Skip prompt
        float blk = 0.5f+0.5f*sinf(t*4.f);
        DrawText("[ any key to skip ]", SW/2-MeasureText("[ any key to skip ]",10)/2, (int)(cy2+44.f), 10,
                 {0,255,200,(unsigned char)(blk*oa*.55f)});
    }
    // Tutorial hint (progressive, after overlay dismissed)
    if (g.currentStage==1 && g.screen==Screen::Play && (g.tutorialMask & 0x80)) {
        const char* hint=nullptr;
        if      ((g.tutorialMask& 1)&&!(g.tutorialMask& 2)) hint="DOUBLE JUMP: press jump again mid-air";
        else if ((g.tutorialMask& 2)&&!(g.tutorialMask& 4)) hint="WALL JUMP: jump while pressing into a wall";
        else if ((g.tutorialMask& 4)&&!(g.tutorialMask& 8)) hint="Stomp enemies from ABOVE to defeat them!";
        else if ((g.tutorialMask& 8)&&!(g.tutorialMask&16)) hint="MUSHROOM = SUPER mode: absorbs one hit!";
        if (hint) {
            int hw2=MeasureText(hint,11);
            DrawRectangle(SW/2-hw2/2-12,SH-78,hw2+24,20,{5,8,20,190});
            DrawRectangleLinesEx({(float)(SW/2-hw2/2-12),(float)(SH-78),(float)(hw2+24),20},1.f,{0,255,200,55});
            DrawText(hint,SW/2-hw2/2,SH-75,11,{0,255,200,225});
        }
    }
    drawCloseBtn();
    if (drawBtn(SW-102,SH-34,92,28,"MENU",{5,8,18,200},{0,20,15,240},15))
        { g.screen=Screen::Menu; stopAllBGM(); }

    if (g.flashTimer > 0.f) {
        DrawRectangle(0,0,SW,SH,{231,76,60,(unsigned char)(std::min(g.flashTimer*400.f,120.f))});
    }
    if (g.fadeAlpha > 0.f) DrawRectangle(0,0,SW,SH,{0,0,0,(unsigned char)(g.fadeAlpha*255.f)});

    if (g.screen==Screen::Pause)         { drawPause(); return; }
    if (g.screen==Screen::LevelComplete) { drawLevelComplete(); return; }
    if (g.screen==Screen::StageMap)      { drawStageMap(); return; }
    if (g.screen==Screen::StageComplete) { drawStageComplete(); return; }
    if (g.screen==Screen::GameWinner)    { drawGameWinner(); return; }
    if (g.screen==Screen::Play)          return;

    // Game Over overlay
    DrawRectangle(0,0,SW,SH,{0,0,0,175});
    // Scanline-like horizontal glitch lines
    for (int i=0;i<6;i++) {
        float gy=fmodf(i*77.f+t*28.f,(float)SH);
        DrawRectangle(0,(int)gy,SW,2,{200,20,20,(unsigned char)(25+i*8)});
    }
    // Chromatic aberration title
    float bsc = 1.f + sinf(t*2.f) * .05f;
    const char* title2 = "GAME OVER";
    int fs=54, tw3=MeasureText(title2,fs);
    int tx=(int)(SW/2-tw3*bsc/2), ty=(int)(SH/2-78-(bsc-1)*12);
    DrawText(title2,tx+4,ty+2,fs,{180,0,0,80});
    DrawText(title2,tx-3,ty,  fs,{255,30,30,90});
    drawNeonText(title2,tx,ty,fs,{220,40,40,255});

    // Stats panel
    char stgBuf[40]; snprintf(stgBuf,sizeof(stgBuf),"Reached: Stage %d - Level %d",g.currentStage,g.levelInStage);
    DrawText(stgBuf,SW/2-MeasureText(stgBuf,16)/2,SH/2-12,16,{180,200,255,220});
    std::string sc2 = "Score: " + std::to_string(g.score);
    DrawText(sc2.c_str(),SW/2-MeasureText(sc2.c_str(),18)/2,SH/2+14,18,WHITE);
    char dBuf[32]; snprintf(dBuf,sizeof(dBuf),"Deaths: %d",g.totalDeaths);
    DrawText(dBuf,SW/2-MeasureText(dBuf,14)/2,SH/2+38,14,{255,120,120,200});
    if (g.highScore>0){std::string hs="Best Score: "+std::to_string(g.highScore);
        DrawText(hs.c_str(),SW/2-MeasureText(hs.c_str(),13)/2,SH/2+60,13,{255,215,0,180});}

    if (drawBtn(SW/2-112,SH/2+88,106,40,"RESTART",{39,174,96,255},{46,204,113,255},18)) resetGame();
    if (drawBtn(SW/2+12, SH/2+88,100,40,"MENU",   {41,128,185,255},{52,152,219,255},18))
        { g.screen=Screen::Menu; stopAllBGM(); }
    DrawText("(or press R)",SW/2-MeasureText("(or press R)",11)/2,SH/2+136,11,{140,140,140,180});
}

// ── Level Editor ──────────────────────────────────────────────────────────────
static const char*  ED_TOOL_NAMES[]  = {"PLATFORM","COIN","ENEMY","MUSHROOM","CHECKPOINT","ERASE"};
static const Color  ED_TOOL_COLORS[] = {{80,80,200,255},{241,196,15,255},{192,57,43,255},
                                        {231,152,15,255},{39,174,96,255},{180,50,50,255}};
static float edDragX = 0, edDragY = 0;
static bool  edDragging = false;
static char  edStatus[64] = "";

static void editorSave() {
    char fn[32]; snprintf(fn, sizeof(fn), "level_%d_edit.txt", g.currentLevel);
    std::ofstream f(fn);
    if (!f) { snprintf(edStatus, sizeof(edStatus), "SAVE FAILED"); return; }
    f << "LEVEL " << g.currentLevel << "\n";
    for (const auto& p : g.platforms)
        f<<"PLAT "<<(int)p.x<<" "<<(int)p.y<<" "<<(int)p.w<<" "<<(int)p.h<<" "<<p.vx<<" "<<p.vy<<" "<<(int)p.bound0<<" "<<(int)p.bound1<<"\n";
    for (const auto& c : g.coins)
        f<<"COIN "<<(int)c.x<<" "<<(int)c.y<<"\n";
    for (const auto& e : g.enemies)
        f<<"ENEMY "<<(int)e.x<<" "<<(int)e.y<<" "<<e.vx<<" "<<e.vy<<" "<<(int)e.x0<<" "<<(int)e.x1<<" "<<(int)e.y0<<" "<<(int)e.y1<<" "<<e.type<<" "<<e.health<<"\n";
    for (const auto& m : g.mushrooms)
        f<<"MUSH "<<(int)m.x<<" "<<(int)m.y<<"\n";
    for (const auto& cp : g.checkpoints)
        f<<"CP "<<(int)cp.x<<" "<<(int)cp.y<<"\n";
    snprintf(edStatus, sizeof(edStatus), "Saved %s", fn);
}

void drawEditor() {
    if (IsKeyDown(KEY_RIGHT)) g.editorCameraX = std::min(g.editorCameraX+6.f, g.levelWidth-(float)SW);
    if (IsKeyDown(KEY_LEFT))  g.editorCameraX = std::max(g.editorCameraX-6.f, 0.f);
    g.cameraX = g.editorCameraX;

    for (int i = 0; i < 6; i++) if (IsKeyPressed(KEY_ONE+i)) g.editorTool = i;
    if (IsKeyPressed(KEY_E)) g.editorTool = 5;

    DrawRectangleGradientV(0,0,SW,SH,skyTop(),skyBot());
    drawStars(g.editorCameraX);
    drawHorizonSun(g.editorCameraX);
    drawPerspectiveGrid(g.editorCameraX);
    drawTrees(g.editorCameraX); drawClouds(g.editorCameraX);

    Camera2D cam = {}; cam.target = {g.editorCameraX, 0}; cam.zoom = 1.f;
    BeginMode2D(cam);

    for (const auto& p  : g.platforms)   drawPlatform(p);
    for (const auto& c  : g.coins)        if (!c.collected)  DrawCircle((int)c.x,(int)c.y,8,{241,196,15,255});
    for (const auto& m  : g.mushrooms)    if (!m.collected)  DrawCircle((int)m.x,(int)m.y,8,{192,57,43,255});
    for (const auto& cp : g.checkpoints)
        DrawTriangle({cp.x+5,cp.y-24},{cp.x+5,cp.y-8},{cp.x+18,cp.y-16},{39,174,96,255});
    for (const auto& e  : g.enemies)      if (e.alive) DrawRectangle((int)e.x,(int)e.y,28,28,e.col);
    drawFlag();

    Color grid = {255,255,255,18};
    for (float gx = fmodf(-g.editorCameraX,32.f); gx < SW; gx += 32)
        DrawLineEx({gx+g.editorCameraX,0},{gx+g.editorCameraX,(float)SH},0.5f,grid);
    for (float gy = 0; gy < SH; gy += 32)
        DrawLineEx({g.editorCameraX,gy},{g.editorCameraX+(float)SW,gy},0.5f,grid);

    Vector2 mp = g.mouse;
    float wx = mp.x + g.editorCameraX, wy = mp.y;
    float snap = 16.f;
    float snx = roundf(wx/snap)*snap, sny = roundf(wy/snap)*snap;

    if (g.editorTool == 0) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { edDragX=snx; edDragY=sny; edDragging=true; }
        if (edDragging) {
            float pw = std::max(snap, fabsf(snx-edDragX)), ph = std::max(snap, fabsf(sny-edDragY));
            float px2 = std::min(snx,edDragX), py2 = std::min(sny,edDragY);
            DrawRectangle((int)px2,(int)py2,(int)pw,(int)ph,{100,150,255,100});
            DrawRectangleLinesEx({px2,py2,pw,ph},1.f,{200,200,255,200});
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                g.platforms.push_back({px2,py2,pw,ph});
                edDragging = false;
                snprintf(edStatus,sizeof(edStatus),"Added platform %.0fx%.0f at %.0f,%.0f",pw,ph,px2,py2);
            }
        }
    } else if (g.editorTool == 5) {
        DrawCircle((int)wx,(int)wy,14,{200,50,50,80});
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            g.platforms.erase(std::remove_if(g.platforms.begin(),g.platforms.end(),
                [wx,wy](const Platform& p){return wx>=p.x&&wx<=p.x+p.w&&wy>=p.y&&wy<=p.y+p.h;}),g.platforms.end());
            g.coins.erase(std::remove_if(g.coins.begin(),g.coins.end(),
                [wx,wy](const Coin& c){return fabsf(c.x-wx)<14&&fabsf(c.y-wy)<14;}),g.coins.end());
            g.mushrooms.erase(std::remove_if(g.mushrooms.begin(),g.mushrooms.end(),
                [wx,wy](const Mushroom& m){return fabsf(m.x-wx)<14&&fabsf(m.y-wy)<14;}),g.mushrooms.end());
            g.checkpoints.erase(std::remove_if(g.checkpoints.begin(),g.checkpoints.end(),
                [wx,wy](const Checkpoint& cp){return fabsf(cp.x-wx)<18&&fabsf(cp.y-wy)<30;}),g.checkpoints.end());
            g.enemies.erase(std::remove_if(g.enemies.begin(),g.enemies.end(),
                [wx,wy](const Enemy& e){return wx>=e.x&&wx<=e.x+28&&wy>=e.y&&wy<=e.y+28;}),g.enemies.end());
            snprintf(edStatus,sizeof(edStatus),"Erased at %.0f,%.0f",wx,wy);
        }
    } else {
        DrawCircle((int)snx,(int)sny,8,ED_TOOL_COLORS[g.editorTool]);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if      (g.editorTool==1) g.coins.push_back({snx,sny,false});
            else if (g.editorTool==2) g.enemies.push_back({snx,sny,1.4f,0,snx-80,snx+80,sny,0,true,{192,57,43,255},0,0,0,0.f});
            else if (g.editorTool==3) g.mushrooms.push_back({snx,sny,false});
            else if (g.editorTool==4) g.checkpoints.push_back({snx,sny,false});
            snprintf(edStatus,sizeof(edStatus),"Placed %s at %.0f,%.0f",ED_TOOL_NAMES[g.editorTool],snx,sny);
        }
    }

    EndMode2D();

    if (IsKeyPressed(KEY_S)) editorSave();

    DrawRectangle(0,0,SW,36,{0,0,0,200});
    DrawText("LEVEL EDITOR",8,10,16,{255,215,0,255});
    for (int i = 0; i < 6; i++) {
        float bx = 120.f + i*110.f;
        bool sel = (g.editorTool == i);
        DrawRectangleRounded({bx,4,104,28},0.3f,6,sel?ED_TOOL_COLORS[i]:Color{50,50,50,200});
        if (sel) DrawRectangleRoundedLinesEx({bx,4,104,28},0.3f,6,2.f,WHITE);
        char label[24]; snprintf(label,sizeof(label),"%d: %s",i+1,ED_TOOL_NAMES[i]);
        DrawText(label,(int)bx+4,11,12,WHITE);
    }
    DrawRectangle(0,SH-22,SW,22,{0,0,0,180});
    char hud[128];
    snprintf(hud,sizeof(hud),"Arrows:Pan | S:Save | F2/ESC:Exit | World(%.0f,%.0f) | Plats:%d Coins:%d Ens:%d",
        wx,wy,(int)g.platforms.size(),(int)g.coins.size(),(int)g.enemies.size());
    DrawText(hud,6,SH-17,11,{180,180,180,220});
    if (edStatus[0]) DrawText(edStatus,SW-MeasureText(edStatus,12)-8,SH-17,12,{100,255,150,220});
    drawCloseBtn();
}
