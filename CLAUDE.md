# CLAUDE CODE MASTER BRIEF — "SURGE"
# C++ / raylib 2D platformer. Save this entire brief to CLAUDE.md verbatim 
# before doing anything else.

## ROLE
You are my senior game programmer + technical art director.
We are taking an existing working platformer from 7/10 to 10/10.
The bar is: "a senior game developer plays this and says 'this feels real.'"

## NON-NEGOTIABLE PRINCIPLES
1. Speed is the feeling. Every system must amplify the sensation of going fast.
2. Polish > content. Few great levels beat many mediocre ones.
3. Every change must be visible in gameplay, not just in code.
4. Ship-stable > feature-complete. A crash kills the project.
5. No regressions. After every change: build clean, run, verify.

## GAME IDENTITY
Working title: VELOCITY (suggest 3 better names after reading the codebase)
Genre: Fast-paced 2D action platformer
Tagline: "Chain the speed. Break the level."
Feel inspirations: Celeste's tight controls, Geometry Dash's flow,
Hotline Miami's lethality, N++'s precision, Hyper Light Drifter's dash.

## SIGNATURE MECHANIC — BUILD THIS FIRST: CHAIN-DASH MOMENTUM

This is the game's identity. Nothing else matters until this feels incredible.

- Player has a dash: 8-directional, ~3 tiles distance, ~0.15s duration,
  i-frames during dash, slight after-image trail
- Dash refreshes on: ground touch, dash-killing an enemy, collecting an orb
- CHAIN MULTIPLIER: each enemy killed within 1.5s of last kill stacks
  - Chain 1-2:  white particles, normal shake
  - Chain 3-5:  cyan particles, +1 frame hit-stop, stronger shake
  - Chain 6-9:  magenta particles, +2 frame hit-stop, vignette pulse
  - Chain 10+:  rainbow trail, time slows to 0.7x for 0.5s, MAX shake,
                pitch-shifted sound, on-screen "INSANE!" popup
- Chain breaks on: damage taken, 1.5s with no kill, prolonged ground-slow
- Combo counter floats on screen, grows in size with chain depth
- Tune across multiple play sessions until it feels addictive

## VISUAL DIRECTION — NEON / SYNTHWAVE (code-drawn, zero external assets)

- Palette:
  - Background:   #1a0033 (deep purple)
  - Player:       #00f5ff (neon cyan) with glow
  - Enemies:      #ff006e (hot pink)
  - Highlights:   #ffffff (white)
  - Collectibles: #ffbe0b (yellow)
- Player: simple geometric shape (triangle or rounded square) with glow + trail
- Enemies: distinct silhouettes — circle (chaser), diamond (static spike),
  hexagon (shooter), big circle (mini-boss)
- Background: parallax grid lines + horizon synthwave sun + slow scroll
- Fake bloom: draw shape, then larger blurred copy underneath at low alpha
- Every collectible has halo + rotation + bob animation

## GAME FEEL TARGETS — implement ALL of these, no exceptions

1.  Hit-stop: 3 frames on enemy kill, 5 frames on player damage
2.  Screen shake: scaled by event magnitude, Vector2 noise, smooth decay
3.  Screen flash: 2 frames white on damage, 1 frame on big chain
4.  Coyote time: 6 frames (~100ms) — jump still works just after leaving ledge
5.  Jump buffer: 9 frames (~150ms) — early jump press registers on landing
6.  Variable jump height: release jump early = shorter jump
7.  Squash & stretch: player scales on jump (1.2y/0.8x), squash on land (0.8y/1.2x)
8.  Speed lines: when player moves >70% max speed, draw motion lines behind
9.  Camera lookahead: leads in direction of movement, smooth-damp
10. Death particles: enemy explodes into 8-12 particles with gravity + drag
11. Pickup feedback: score popup floats up + fades, sound pitch rises per combo
12. Landing dust: small puff on land, intensity = fall speed
13. Damage flash: enemy white-flashes 2 frames before dying
14. Input feel: stick dead zone, non-linear response curve

## ARCHITECTURE WORK (refactor existing code, do NOT rewrite from scratch)

After signature mechanic is in, do these in order:

1. Fixed timestep loop: 60Hz physics with interpolated render
   - Standard accumulator pattern
   - Input polling stays per-frame
2. RAII wrappers for raylib Sound and Music — kill the manual UnloadSound block
3. Save file versioning for records.txt and settings.txt
   - First line = version int
   - Migrate v1 → v2 forward
   - Corrupt/newer file → back up to .bak, start fresh
4. Tuning.h — every magic number (timers, speeds, particle counts, colors)
   named constexpr in one file
5. Letterboxed virtual resolution: render to RenderTexture at SW×SH, scale
   to window with black bars. Use FLAG_WINDOW_RESIZABLE.
6. ScreenManager class — each screen (Menu, Play, Pause, LevelComplete,
   StageMap, StageComplete, GameWinner, Settings, Edit, Over, Win) owns
   update(dt) / draw() / onEnter() / onExit(). main.cpp just calls
   screenManager.tick(dt).
7. Particle system — single pre-allocated pool (500 particles max),
   zero allocations per frame, used by all VFX
8. Event bus — decouple combat → effects → audio → UI. No tight coupling.

## SHIP-STABLE CHECKLIST (everything must work before final commit)

- [ ] Pause menu: Resume / Settings / Quit to Menu
- [ ] Settings: master/music/sfx volume, fullscreen toggle, key rebind
- [ ] Gamepad fully supported (already partial)
- [ ] Save survives version bumps
- [ ] Window resize doesn't break UI (letterbox handles it)
- [ ] No crash on missing files — fall back to defaults
- [ ] Quit saves automatically
- [ ] Death counter + time tracker on results screen
- [ ] First-run tutorial: short, no text, just symbol prompts
      (arrow = move, space = jump, shift = dash). Skippable.
- [ ] No memory leaks (verify with an extended play session)
- [ ] Stable 60fps on integrated graphics

## CONTENT TARGETS (cut scope ruthlessly)

- 3 stages × 5 levels = 15 hand-tuned levels (NOT the existing 100)
- 1 boss per stage = 3 bosses total
- 4 enemy types: chaser, static-spike, shooter, mini-boss
- 6 BGM tracks (keep the existing ones)
- 1 unlockable: Hard Mode after first clear

Cut existing levels aggressively. Better 15 great than 100 mediocre.

## WORKFLOW RULES

1. Always enter Plan Mode for non-trivial changes
2. Show the plan, wait for my approval, then execute
3. After every system: build clean, run, confirm no regression
4. Commit per feature with conventional commit messages
   (feat:, fix:, refactor:, chore:)
5. If you hit ambiguity, ASK — don't guess
6. If a feature is large, propose a v1/v2 split before starting
7. Track progress in TODO.md, update after each session
8. Never delete code without explaining why in the commit message

## EXECUTION ORDER (do phases in this sequence, fully complete each before next)

Phase 1: Read codebase, save CLAUDE.md, refactor ScreenManager + Tuning.h
Phase 2: Chain-dash mechanic + particle pool + all 14 game-feel items
Phase 3: Fixed timestep + RAII + save versioning + virtual resolution
Phase 4: Visual overhaul → neon/synthwave aesthetic + fake bloom + parallax
Phase 5: Cut to 15 levels, hand-tune each, build 3 bosses
Phase 6: Audio pass — re-mix BGM, chain-stack sound design, pitch ramps
Phase 7: Event bus refactor + ship-stable checklist sweep
Phase 8: First-run tutorial + settings polish + key rebind
Phase 9: Playtest pass — find feel issues, tune numbers in Tuning.h
Phase 10: Final bug fixes, performance pass, build verification

## START HERE — DO NOT WRITE CODE YET

1. Read these files completely: main.cpp, game.cpp, game.h, types.h,
   globals.h, levels.cpp, levels.h, procgen.cpp, procgen.h, render.cpp,
   render.h, audio.cpp, audio.h, AudioManager.h, ResourceManager.cpp,
   ResourceManager.h.
2. Save this entire brief verbatim to CLAUDE.md.
3. Produce CURRENT_STATE.md with:
   - What's already great (be specific, name files/functions)
   - What's broken, risky, or technically debt-y
   - What's missing for the vision above
   - Honest assessment of scope: what should be cut or simplified
4. Propose 3 better names than "VELOCITY" based on actual gameplay.
5. Stop and wait for my approval before ANY code changes.

Confirm understanding by listing the 3 things you'll do first, then begin.
