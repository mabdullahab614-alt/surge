# CURRENT STATE AUDIT
Generated after full codebase read. No code has been changed.

---

## WHAT'S ALREADY GREAT

**audio.cpp — `waveToLoopingMusic()`**
Genuinely clever: synthesises 6 full BGM tracks from pure math (square + triangle
waves + Schroeder reverb), packs them into heap-allocated in-memory WAV buffers,
and streams them with no file I/O. The malloc-intentional-leak pattern is correctly
documented. This is production-quality for a no-asset game. Keep all 6 tracks.

**game.cpp — movement feel**
`update()` has: dt-normalized physics (DT60 pattern), 6-frame coyote time,
8-frame jump buffer, wall jump with cooldown, double jump, dash with afterimage.
These are the right primitives. They just need tuning and the chain-dash extension.

**procgen.cpp — `generateLevel()`**
Deterministic xorshift32 seeded by `stage * 73856093u ^ level * 19349663u`.
9 generation passes (ground, floats, coins, enemies, boss, mushrooms, checkpoints,
spikes, geysers). Produces reproducible, diff-scaled content. Architecture is solid.

**render.cpp — visual consistency**
Consistent neon-cyberpunk aesthetic throughout. Procedural tile textures (7 types,
GenImageColor pixel-by-pixel). Fake bloom via concentric circles at low alpha —
exactly the right technique for zero-asset. `drawNeonText`, `drawCornerBrackets`,
`drawCyberGrid` are reusable helpers. All screens use the same palette.

**globals.h — single GameState**
One global struct, all state in one place, clean extern. No scattered globals.
Easy to serialize, easy to inspect, no hidden state.

**Game loop completeness**
Grade system (S/A/B/C), death counters, level scorecard, stage map with grade
badges, save/load for grades+progress, 3.5s auto-advance — the full game loop
exists and works.

---

## WHAT'S BROKEN, RISKY, OR DEBT-Y

### Crash risks
**ResourceManager.h lines 51-53** — `extern template` declarations with no
matching explicit instantiations anywhere. Would cause linker errors if these
headers were ever included in a translation unit that tries to use them.
ResourceManager.cpp is also NOT listed in CMakeLists.txt sources, so it doesn't
compile. These files are completely vestigial — included in no .cpp, never used.
**Action: delete or quarantine before Phase 3 architecture work.**

**AudioManager.h** — Same issue. Never `#include`d anywhere in the codebase.
Static maps with no .cpp counterpart. Dead code.

### Physics correctness
**No fixed timestep.** Uses raw `dt = GetFrameTime()` for all physics.
A 2-frame lag spike at 30fps = double-speed physics that frame.
Platform collision can tunnel through thin platforms at low FPS.
hitStop is measured in frame counts (`int hitStop`), which means a 10fps frame
makes hitstop last 6× longer than intended.

**`hitStop` is frame-count, not time.** `if (g.hitStop > 0) { g.hitStop--; return; }`
At variable frame rate this produces inconsistent feel. At 120fps hitstop is
imperceptibly short; at 30fps it's way too long.

### Technical debt
**`levels.h` is stale.** Declares `mkPlats1()` through `mkPlats6()` — old
hand-coded level functions from before procgen. These functions no longer exist
anywhere. The header still ships. Any new file that includes it will fail to link.

**`update()` is 460+ lines.** One function handles: input, dash, wall-jump,
double-jump, moving platforms, collision, hazards (spikes, geysers, lava),
checkpoints, mushrooms, coins, flag, enemy AI (5 types), projectiles, camera,
tutorial, particles, popups. Untestable in isolation, hard to read.

**`render.cpp` is 1400+ lines.** Everything from tile textures to HUD to
level-complete overlay in one file. `drawScene()` alone is 230+ lines.

**Magic numbers scattered everywhere.** GRAVITY, JUMP_FORCE, MOVE_SPEED are
in types.h (good), but 0.15f (dash duration), 1.5f (combo timer), 0.9f (camera
decay), 3.2f (vanish cycle), 300/400 (geyser heights), 2.5f (stage map delay)
are all buried inline with no names. Tuning.h doesn't exist yet.

**Save file has no version header.** After our last session we extended records.txt
with 100 levelGrade values. An old save file silently reads garbage into
`levelGrades[][]`. A future format change will silently corrupt existing saves again.

**`LEVEL_WIDTHS[]` in types.h** — defined as `static const float LEVEL_WIDTHS[] =
{0.f, 3200.f, 3800.f, ...}` but never read anywhere. procgen calculates its own
width. Dead constant.

**Particle system uses `std::vector` with per-frame push_back/erase.** Each
`spawnParticles()` call heap-allocates. On a big chain kill: 30 particles × 3
`malloc`s = ~90 small heap ops in one frame. Not a problem at current scale but
blocks zero-alloc target.

### Architecture gaps
**No ScreenManager.** Screen transitions are if-chains in main.cpp (lines 109-156)
and direct `g.screen =` assignments scattered across game.cpp (17 assignments),
main.cpp (8 assignments). Impossible to add entry/exit hooks cleanly.

**No EventBus.** `update()` in game.cpp directly calls `spawnParticles()`,
`playSFX()`, `shake()`, `spawnPopup()`, `unlockAch()`, `saveRecords()` all inline.
Combat is tightly coupled to VFX, audio, and persistence simultaneously.

**No virtual resolution / letterbox.** `InitWindow(SW, SH, ...)` at 800×450 fixed.
Fullscreen on a 2560×1440 monitor runs at 800×450 in a window, or stretches with
incorrect aspect ratio. FLAG_WINDOW_RESIZABLE not set.

---

## WHAT'S MISSING FOR THE VISION

### Signature mechanic (entire thing is missing)
- **8-directional dash** — current dash is horizontal-only. No vertical or diagonal.
- **Dash-kill** — dash currently only moves the player; enemies are immune to it.
  There is no concept of a "dash kill" that refreshes the dash.
- **Chain multiplier escalation** — the existing combo system counts stomps within
  1.8s but has none of the visual/audio escalation tiers (cyan → magenta → rainbow).
- **Time-slow on chain 10+** — not present.
- **Dash refresh on kill / orb collect** — not present.
- **After-image trail on dash** — partially exists (single ellipse behind player)
  but not the multi-frame ghost trail called for.

### Game feel (from the 14 targets)
- Variable jump height (release early = shorter): **MISSING.** Jump force is
  applied once on press, never reduced on release.
- Speed lines at >70% max speed: **MISSING.**
- Camera lookahead leading movement direction: **PARTIAL.** Camera leads by
  `vx * 32f` which is a fixed offset, not a smooth damp to a lookahead target.
- Non-linear gamepad response curve: **MISSING** (linear axis pass-through).
- Damage flash on enemy (2 frames white before death): **MISSING.** Enemies just
  disappear or play deathTimer animation.

Already present (good): hit-stop, screen shake, screen flash, coyote time,
jump buffer, squash+stretch, death particles, pickup feedback, landing dust.

### Visual direction
- **Synthwave aesthetic is not implemented.** Current look is neon cyberpunk
  (dark background, cyan lines, grid). The spec calls for synthwave: deep purple
  `#1a0033` background, horizon gradient sun, parallax grid lines, player as
  geometric shape.
- **Player shape**: currently a detailed pixel-art humanoid (hat, shirt, pants, shoes).
  Spec: geometric rounded-square or triangle with glow.
- **Enemy shapes**: currently varied cartoon sprites. Spec: circle/diamond/hexagon/
  big-circle silhouettes.
- **Fake bloom**: exists for some elements but not systematic — not every emitting
  object has a larger blurred copy underneath.

### Architecture
- Fixed timestep loop: **MISSING**
- RAII Sound/Music wrappers: **MISSING** (8-line manual Unload block in main.cpp)
- Save file versioning: **MISSING**
- Tuning.h: **MISSING**
- Letterboxed virtual resolution: **MISSING**
- ScreenManager: **MISSING**
- Pre-allocated particle pool: **MISSING**
- EventBus: **MISSING**

### Content
- 15 hand-tuned levels: **MISSING** (have 100 procedural)
- 3 distinct bosses with unique behaviors: **MISSING** (1 boss type, color variants)
- Hard Mode unlock after first clear: **MISSING** (Hard Mode exists but is just
  a settings toggle, not an unlock)
- First-run tutorial (symbol-based, skippable): **PARTIAL** (text hints only on S1L1)
- Key rebinding: **MISSING**

---

## HONEST SCOPE ASSESSMENT

**What to cut or simplify to ship:**

The architecture phase (Phase 3) is the highest risk. ScreenManager + EventBus
together are a 3-4 day full refactor of both game.cpp and main.cpp. They make the
codebase better but add zero visible gameplay. Recommend a v1/v2 split:
- **v1** (Phase 1-2): Chain-dash + all 14 game-feel items + synthwave visuals.
  These are immediately visible and make the game feel completely different.
- **v2** (Phase 3+): Architecture cleanup. Do it after the fun is confirmed.

**Fixed timestep is non-negotiable** for the physics-precision feel (Celeste
reference). Do this in Phase 3 before content.

**Cut: EventBus for now.** Loose coupling is nice but the game is small enough
that tight coupling in game.cpp is manageable. Revisit in Phase 7.

**Cut: ResourceManager and AudioManager** — delete both files entirely.
They're unused stubs that add confusion and one has broken extern templates.

**Cut: ScreenManager for now.** Instead, a minimal refactor: extract each screen's
update/draw into its own function pair and centralize transitions in main.cpp.
Full OOP ScreenManager is architecturally over-engineered for this project size.

**The 100-level → 15-level cut is the right call** but is a large behavioral
change. The procgen system should be kept in source (don't delete it) but
disabled. The 15 levels need to be hand-authored in a new levels_handmade.cpp.

**Feasible in reasonable sessions:** Phases 1-4 (chain-dash, game-feel, fixed
timestep, synthwave visuals). Phases 5-10 depend on playtesting time.

---

## THREE BETTER NAMES THAN "VELOCITY"

**1. KINETICA**
Kinetic energy + synthetic (synth aesthetic) + the -ica suffix gives it an
identity, sounds like a product, looks great as a neon title. "KINETICA" on a
purple synthwave screen with glow immediately reads as a game name.

**2. SLIPSTREAM**
The aerodynamic wake behind a fast-moving object. Directly evokes: speed, chaining
through enemies (slipping through them), the trail effect, momentum. One word,
evocative, sounds fast. "Chain the speed" matches perfectly.

**3. SURGE**
Short, electric, punchy. Synthwave aesthetic = electricity = surge. The chain-dash
chain is a surge of momentum. Three-frame-hitstop kills feel like a surge of energy.
Works as a title, in a tagline, on a leaderboard ("SURGE // WORLD RECORD").

**Recommendation: SURGE.** Four letters, impossible to mispronounce, iconic at
any font size, pairs with the tagline perfectly.

---

## STOP — AWAITING APPROVAL

No code has been changed. Waiting for your go-ahead on:
1. Which name (or keep VELOCITY / suggest your own)
2. Confirm execution order (start with Phase 1: chain-dash first, or architecture first)
3. Any cuts from the scope above you want to keep or restore
