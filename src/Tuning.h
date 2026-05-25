#pragma once

// ── Phase 2: Dash ─────────────────────────────────────────────────────────────
constexpr float DASH_DURATION    = 0.15f;
constexpr float DASH_SPEED_MULT  = 3.2f;
constexpr float DASH_COOLDOWN    = 0.6f;
constexpr int   DASH_IFRAMES     = 9;
constexpr int   AFTERIMAGE_COUNT = 5;

// ── Phase 2: Chain multiplier ─────────────────────────────────────────────────
constexpr float CHAIN_WINDOW        = 1.5f;
constexpr float CHAIN_TIMESCALE     = 0.7f;
constexpr float CHAIN_TIMESCALE_DUR = 0.5f;

// ── Phase 2: Jump / movement feel ────────────────────────────────────────────
constexpr float JUMP_CUT_MULT    = 0.45f;
constexpr float SPEEDLINE_THRESH = 0.70f;
constexpr float CAM_LOOKAHEAD_VX = 45.f;
constexpr float CAM_LOOKAHEAD_VY = 18.f;

// ── Phase 3: Fixed timestep ───────────────────────────────────────────────────
constexpr float FIXED_DT = 1.f / 60.f;
