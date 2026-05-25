#include "procgen.h"
#include <cmath>
#include <algorithm>

// ── Stage metadata ────────────────────────────────────────────────────────────

int stageTheme(int stage) {
    // Stages 1-3: grass(1)/caves(3)/volcano(4); rest kept for procgen fallback
    static const int MAP[] = {0, 1, 3, 4, 4, 5, 6, 4, 3, 5, 6};
    return MAP[stage < 1 ? 1 : stage > 10 ? 10 : stage];
}

bool isIceStage(int stage) { return stage == 5 || stage == 9; }

const char* stageName(int stage) {
    static const char* NAMES[] = {
        "", "Grasslands", "Crystal Caves", "Volcano Peak",
        "Volcano Peak", "Ice Kingdom", "Sky Fortress",
        "Haunted Grove", "Shadow Depths", "Inferno Abyss", "Final Citadel"
    };
    return (stage >= 1 && stage <= 10) ? NAMES[stage] : "Unknown";
}

// ── Deterministic RNG (xorshift32) ───────────────────────────────────────────

static uint32_t rng_state;

static void rng_seed(int stage, int level) {
    rng_state = (uint32_t)(stage * 73856093u ^ level * 19349663u ^ 2654435761u);
    if (rng_state == 0) rng_state = 1;
}

static uint32_t rng_next() {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static float rng_f(float lo, float hi) {
    return lo + (float)(rng_next() % 100000u) / 100000.f * (hi - lo);
}

static int rng_i(int lo, int hi) {
    if (hi <= lo) return lo;
    return lo + (int)(rng_next() % (unsigned)(hi - lo + 1));
}

static bool rng_pct(int pct) { return (int)(rng_next() % 100u) < pct; }

// ── Enemy color palette per stage ────────────────────────────────────────────

static Color enemyColors[11][3] = {
    {},
    {{192,57,43,255},  {41,128,185,255},  {142,68,173,255}},   // 1 Grass
    {{39,174,96,255},  {80,20,80,255},    {142,68,173,255}},    // 2 Forest
    {{80,20,80,255},   {142,68,173,255},  {41,128,185,255}},    // 3 Caves
    {{210,90,15,255},  {192,57,43,255},   {142,68,173,255}},    // 4 Volcano
    {{140,200,240,255},{41,128,185,255},  {142,68,173,255}},    // 5 Ice
    {{50,20,80,255},   {180,140,20,255},  {41,128,185,255}},    // 6 Sky
    {{39,100,50,255},  {60,20,80,255},    {160,80,20,255}},     // 7 Haunted Grove
    {{30,30,80,255},   {80,10,80,255},    {20,80,80,255}},      // 8 Shadow Depths
    {{200,50,10,255},  {160,20,10,255},   {120,10,10,255}},     // 9 Inferno
    {{50,10,50,255},   {100,40,140,255},  {180,140,20,255}},    // 10 Final
};

static Color bossColors[11] = {
    {},
    {180,30,30,255},   // 1
    {100,8,8,255},     // 2
    {80,10,80,255},    // 3
    {100,8,8,255},     // 4
    {20,40,100,255},   // 5
    {30,10,50,255},    // 6
    {40,20,10,255},    // 7
    {10,10,60,255},    // 8
    {120,10,10,255},   // 9
    {20,5,40,255},     // 10 Final boss — darkest
};

// ── Level generator ───────────────────────────────────────────────────────────

void generateLevel(int stage, int lev,
                   std::vector<Platform>&   plats,
                   std::vector<Coin>&       coins,
                   std::vector<Enemy>&      enemies,
                   std::vector<Mushroom>&   mushs,
                   std::vector<Checkpoint>& cps,
                   std::vector<Spike>&      spikes,
                   std::vector<Geyser>&     geysers,
                   float&                   levelWidth)
{
    rng_seed(stage, lev);

    int diff = (stage - 1) * 10 + (lev - 1); // 0–99
    bool hasBoss = (lev == 10);

    // ── Level dimensions ──────────────────────────────────────────────────────
    float lw = 2800.f + diff * 36.f;   // 2800–6364 px
    levelWidth = lw;

    const float GY   = 400.f;  // ground Y
    const float GH   = 50.f;   // ground thickness

    // ── Difficulty-scaled parameters ─────────────────────────────────────────
    float platWMin = std::max(60.f,  200.f - diff * 1.5f);
    float platWMax = std::max(100.f, 310.f - diff * 2.2f);
    float gapMin   = std::min(50.f  + diff * 0.7f, 170.f);
    float gapMax   = std::min(90.f  + diff * 1.3f, 230.f);
    float fplatW   = std::max(50.f,  140.f - diff * 0.9f);  // floating platform width
    float enemySpd = 1.2f + diff * 0.024f;
    int   enemyPct = std::min(15 + diff, 70);   // chance per ground chunk
    int   movePct  = std::min(diff / 2, 48);    // chance for moving floating plat

    // ── Pass 1: ground chunks ─────────────────────────────────────────────────
    struct Chunk { float x, w; };
    std::vector<Chunk> chunks;

    float cx = 0.f;
    while (cx < lw - 350.f) {
        float pw = rng_f(platWMin, platWMax);
        pw = std::max(pw, 70.f);
        if (chunks.empty()) pw = std::max(pw, 240.f);  // safe spawn zone
        if (hasBoss && cx + pw + gapMax * 2.f >= lw - 400.f)
            pw = std::max(pw, 380.f);                  // boss arena

        chunks.push_back({cx, pw});
        plats.push_back({cx, GY, pw, GH});

        float gap = rng_f(gapMin, gapMax);
        cx += pw + gap;
    }
    // Final platform — always reaches the end and holds the flag
    float finalX = lw - 300.f;
    plats.push_back({finalX, GY, 300.f, GH});

    // ── Pass 2: floating platforms above each ground chunk ────────────────────
    for (int ci = 1; ci < (int)chunks.size(); ci++) {
        const Chunk& ch = chunks[ci];
        int nFloat = rng_i(1, std::min(4, 1 + diff / 18));

        for (int fi = 0; fi < nFloat; fi++) {
            float fw   = std::max(fplatW, rng_f(fplatW, fplatW * 1.6f));
            float fy   = 330.f - (float)rng_i(0, 5) * 28.f;   // 330 down to 190
            float fxOff = rng_f(5.f, std::max(6.f, ch.w - fw - 5.f));
            float fx   = ch.x + fxOff;

            bool moving = rng_pct(movePct);
            float vx = 0, vy = 0, b0 = 0, b1 = 0;
            if (moving) {
                if (rng_pct(50)) {
                    vx = rng_f(1.4f + diff * 0.02f, 2.3f + diff * 0.018f);
                    b0 = ch.x; b1 = ch.x + ch.w;
                } else {
                    vy = rng_f(1.2f + diff * 0.015f, 2.f + diff * 0.014f);
                    b0 = fy - 90.f; b1 = fy + 70.f;
                }
            }
            plats.push_back({fx, fy, fw, 18.f, vx, vy, b0, b1});

            // Coins on floating platform
            int nc = rng_i(1, 3);
            for (int ci2 = 0; ci2 < nc; ci2++)
                coins.push_back({fx + 5.f + ci2 * 22.f, fy - 22.f, false});
        }
    }

    // Ground coins
    for (const auto& ch : chunks) {
        if (ch.x == 0.f) continue;
        int nc = rng_i(0, 3);
        for (int ci2 = 0; ci2 < nc; ci2++)
            coins.push_back({ch.x + 20.f + ci2 * 28.f, GY - 22.f, false});
    }

    // ── Pass 3: enemies ───────────────────────────────────────────────────────
    Color* pal = enemyColors[stage];

    for (int ci = 2; ci < (int)chunks.size(); ci++) {
        const Chunk& ch = chunks[ci];
        if (!rng_pct(enemyPct)) continue;

        int type = 0;
        if (diff >= 20 && rng_pct(25)) type = 1;  // jumper
        if (diff >= 40 && rng_pct(20)) type = 2;  // shooter
        if (diff >= 60 && rng_pct(18)) type = 4;  // advanced

        Color ec = pal[rng_i(0, 2)];
        float ex  = ch.x + ch.w * 0.5f;
        float ey  = GY - 28.f;
        float esp = rng_f(enemySpd * 0.85f, enemySpd * 1.15f);

        if (type == 0 || type == 4) {
            float st = (type == 4) ? rng_f(0.3f, 1.2f) : 0.f;
            enemies.push_back({ex, ey, esp, 0.f, ch.x, ch.x+ch.w, 0.f, 0.f,
                               true, ec, type, st, 0});
        } else if (type == 1) {
            enemies.push_back({ex, GY - 28.f, esp, 0.f,
                               ch.x, ch.x + ch.w, GY - 180.f, GY,
                               true, ec, 1, 0.f, 0});
        } else {  // shooter
            float si = std::max(0.6f, 2.5f - diff * 0.01f);
            enemies.push_back({ex, ey, 0.f, 0.f, ch.x, ch.x+ch.w, 0.f, 0.f,
                               true, ec, 2, si, 0});
        }
    }

    // ── Pass 4: boss on level 10 ──────────────────────────────────────────────
    if (hasBoss) {
        Color bc = bossColors[stage];
        int   bh = 2 + stage / 2;   // 2 health (stage 1) → 7 health (stage 10)
        float bx = finalX + 130.f;
        float by = GY - 60.f;
        float bs = 1.3f + stage * 0.1f + lev * 0.02f;
        enemies.push_back({bx, by, bs, 0.f,
                           finalX + 20.f, finalX + 280.f, 0.f, 0.f,
                           true, bc, 3, 0.f, bh});
    }

    // ── Pass 5: mushrooms (power-ups) — 2-3 scattered ────────────────────────
    {
        int nm = rng_i(2, 3);
        float step = lw / (nm + 1.f);
        for (int i = 0; i < nm; i++) {
            float mx = step * (i + 1);
            // Snap to nearest ground platform
            for (const auto& p : plats) {
                if (p.vx == 0 && p.vy == 0 && p.h > 20 && mx >= p.x && mx <= p.x + p.w) {
                    mushs.push_back({p.x + p.w * 0.5f - 8.f, p.y - 28.f, false});
                    break;
                }
            }
        }
    }

    // ── Pass 6: checkpoints — every ~1200–1500 px of level ───────────────────
    {
        float spacing = std::max(1200.f, lw / 3.5f);
        float cpx = spacing;
        while (cpx < lw - 500.f) {
            for (const auto& p : plats) {
                if (p.vx == 0 && p.vy == 0 && p.h > 20 && cpx >= p.x && cpx <= p.x + p.w) {
                    cps.push_back({p.x + p.w * 0.5f - 8.f, p.y - 28.f, false});
                    break;
                }
            }
            cpx += spacing;
        }
    }

    // ── Pass 7: crystal spikes on platforms (stage 3+) ────────────────────────
    if (stage >= 3) {
        int spikeChance = std::min(10 + (stage - 3) * 5, 40); // 10% stage 3 → 40% stage 10
        for (int ci = 2; ci < (int)chunks.size() - 1; ci++) {
            const Chunk& ch = chunks[ci];
            if (!rng_pct(spikeChance)) continue;
            // Place a spike cluster near the edge of the chunk
            float sw2 = rng_f(24.f, 44.f);
            float sx  = rng_pct(50) ? ch.x + rng_f(4.f, 20.f) : ch.x + ch.w - sw2 - rng_f(4.f, 20.f);
            spikes.push_back({sx, GY, sw2});
        }
        // Also on floating platforms (rarer)
        for (auto& p : plats) {
            if (p.h > 20 || p.vx != 0 || p.vy != 0) continue; // only static thin platforms
            if (!rng_pct(spikeChance / 3)) continue;
            float sw2 = rng_f(14.f, 28.f);
            if (sw2 < p.w * 0.6f)
                spikes.push_back({p.x + rng_f(2.f, p.w - sw2 - 2.f), p.y, sw2});
        }
    }

    // ── Pass 8: geysers in gaps (stages 4, 9, 10) ────────────────────────────
    if (stage == 4 || stage == 9 || stage == 10) {
        int geyserChance = (stage == 10) ? 70 : 50;
        for (int ci = 0; ci + 1 < (int)chunks.size(); ci++) {
            const Chunk& ca = chunks[ci], &cb = chunks[ci + 1];
            float gapStart = ca.x + ca.w, gapEnd = cb.x;
            float gapW = gapEnd - gapStart;
            if (gapW < 40.f) continue;  // gap too small
            if (!rng_pct(geyserChance)) continue;
            float gx       = gapStart + gapW * 0.5f;
            float cooldown = rng_f(2.f, 4.5f);
            geysers.push_back({gx, cooldown, cooldown, false, 0.f});
            // Stagger start so geysers don't all fire at once
            geysers.back().cooldown = rng_f(0.5f, cooldown);
        }
    }

    // ── Pass 9: vanishing platforms (stage 7+) ────────────────────────────────
    if (stage >= 7) {
        int vanishChance = std::min(25 + (stage - 7) * 15, 60);
        for (auto& p : plats) {
            if (p.h > 20 || p.vx != 0 || p.vy != 0) continue; // floating static only
            if (rng_pct(vanishChance)) p.canVanish = true;
        }
    }
}
