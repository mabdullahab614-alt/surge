<div align="center">

<img src="https://capsule-render.vercel.app/api?type=waving&height=260&text=SURGE&fontSize=110&color=0:1a0033,100:0d0026&fontColor=00f5ff&animation=fadeIn&fontAlignY=42&stroke=00f5ff&strokeWidth=2&desc=Chain%20the%20Speed.%20Break%20the%20Level.%20%E2%80%94%20Built%20by%20Abdullah%20Javid&descSize=18&descAlignY=68&descColor=ff006e" width="100%"/>

[![Typing SVG](https://readme-typing-svg.demolab.com?font=Share+Tech+Mono&size=20&duration=2500&pause=800&color=00F5FF&center=true&vCenter=true&repeat=true&width=700&lines=⚡+Chain+Dashes.+Stack+Kills.+Break+Records.;🎮+C%2B%2B+%2B+raylib+%7C+Compiled+to+WebAssembly;👾+3+Stages+·+3+Bosses+·+Hard+Mode+Unlock;🏆+Neon+Synthwave+Platformer+%7C+Play+in+Browser)](https://mabdullahab614-alt.github.io/surge/)

<br/>

[![Play Live](https://img.shields.io/badge/⚡%20PLAY%20LIVE-surge-00f5ff?style=for-the-badge&logoColor=black)](https://mabdullahab614-alt.github.io/surge/)
[![Stars](https://img.shields.io/github/stars/mabdullahab614-alt/surge?style=for-the-badge&color=ff006e&labelColor=1a0033&logo=github)](https://github.com/mabdullahab614-alt/surge/stargazers)
[![Forks](https://img.shields.io/github/forks/mabdullahab614-alt/surge?style=for-the-badge&color=00f5ff&labelColor=1a0033&logo=github)](https://github.com/mabdullahab614-alt/surge/forks)
[![Issues](https://img.shields.io/github/issues/mabdullahab614-alt/surge?style=for-the-badge&color=ff2d2d&labelColor=1a0033)](https://github.com/mabdullahab614-alt/surge/issues)
[![License](https://img.shields.io/github/license/mabdullahab614-alt/surge?style=for-the-badge&color=ff006e&labelColor=1a0033)](LICENSE)
[![Deploy](https://img.shields.io/github/actions/workflow/status/mabdullahab614-alt/surge/deploy.yml?style=for-the-badge&label=CI%2FCD&labelColor=1a0033&color=00f5ff)](https://github.com/mabdullahab614-alt/surge/actions)

<br/>

![C++17](https://img.shields.io/badge/C%2B%2B17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![raylib](https://img.shields.io/badge/raylib%205.5-white?style=flat-square&logo=data:image/png;base64,iVBORw0KGgo=&logoColor=black)
![WebAssembly](https://img.shields.io/badge/WebAssembly-654FF0?style=flat-square&logo=webassembly&logoColor=white)
![Emscripten](https://img.shields.io/badge/Emscripten-3.1.64-00f5ff?style=flat-square)
![GitHub Pages](https://img.shields.io/badge/GitHub%20Pages-1a0033?style=flat-square&logo=github&logoColor=white)
![Zero Assets](https://img.shields.io/badge/assets-zero-ff006e?style=flat-square)

</div>

---

## 🎬 Gameplay

<div align="center">

<video src="assets/gameplay.mp4" autoplay loop muted playsinline width="100%"></video>

| Chain x3 | Chain x10 | Boss Fight |
|:---------:|:---------:|:----------:|
| 🔵 Cyan particles + shake | 🌈 Rainbow trail + time-slow | 💥 Phase transitions + rage |

[![▶ Play Now — see it live](https://img.shields.io/badge/▶%20Play%20Now%20—%20see%20it%20live-1a0033?style=for-the-badge&color=00f5ff)](https://mabdullahab614-alt.github.io/surge/)

</div>

---

## 📑 Table of Contents

- [Gameplay](#-gameplay)
- [Overview](#-overview)
- [Live Demo](#-live-demo)
- [Features](#-features)
- [Controls](#-controls)
- [Enemy Types](#-enemy-types)
- [Boss Roster](#-boss-roster)
- [Tech Stack](#-tech-stack)
- [Quick Start](#-quick-start)
- [File Structure](#-file-structure)
- [Roadmap](#-roadmap)
- [Contributing](#-contributing)
- [License](#-license)
- [Author](#-author)

---

## 🧠 Overview

**SURGE** is a fast-paced neon/synthwave 2D platformer written entirely in **C++17 with raylib** — compiled to **WebAssembly** so it runs instantly in any browser with zero installation.

The core identity is the **Chain-Dash mechanic**: dash through enemies, stack kills within 1.5 seconds, and watch the multiplier explode into rainbow trails, time-slow, and screen-shaking chaos.

> No game engine. No external assets. Every sound is procedurally synthesized. Every visual is code-drawn.

```
╔══════════════════════════════════════════════════════════════╗
║                                                              ║
║   ███████╗██╗   ██╗██████╗  ██████╗ ███████╗               ║
║   ██╔════╝██║   ██║██╔══██╗██╔════╝ ██╔════╝               ║
║   ███████╗██║   ██║██████╔╝██║  ███╗█████╗                 ║
║   ╚════██║██║   ██║██╔══██╗██║   ██║██╔══╝                 ║
║   ███████║╚██████╔╝██║  ██║╚██████╔╝███████╗               ║
║   ╚══════╝ ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚══════╝               ║
║                                                              ║
║        [ CHAIN THE SPEED ]   [ BREAK THE LEVEL ]            ║
╚══════════════════════════════════════════════════════════════╝
```

---

## 🌐 Live Demo

<div align="center">

### ⚡ [mabdullahab614-alt.github.io/surge](https://mabdullahab614-alt.github.io/surge/)

*Works on Desktop — click canvas to focus, then play instantly*

</div>

---

## ✨ Features

<table>
<tr>
<td width="50%">

### ⚡ Chain-Dash Mechanic
- ✅ 8-directional dash with i-frames & afterimage trail
- ✅ Dash refreshes on ground, kill, or orb pickup
- ✅ Chain x1–2: white particles, normal shake
- ✅ Chain x3–5: cyan particles, +hit-stop, big shake
- ✅ Chain x6–9: magenta particles, vignette pulse
- ✅ Chain x10+: rainbow trail, **0.7× time-slow**, "INSANE!" popup

</td>
<td width="50%">

### 🎮 Game Feel
- ✅ Hit-stop: 3 frames on kill, 5 on damage
- ✅ Screen shake scaled by event magnitude
- ✅ Coyote time (6 frames) + Jump buffer (9 frames)
- ✅ Variable jump height (early release = short jump)
- ✅ Squash & stretch on jump and land
- ✅ Speed lines when moving at 70%+ max speed
- ✅ Camera lookahead + smooth-damp

</td>
</tr>
<tr>
<td width="50%">

### 👾 Content
- ✅ 3 Stages × 8 Levels = 24 hand-tuned levels
- ✅ 3 multi-phase bosses with rage modes
- ✅ 4 enemy types with stage-specific designs
- ✅ Hard Mode unlocked after first clear
- ✅ Per-level grade system + best times
- ✅ Death counter + run timer on results screen

</td>
<td width="50%">

### 🔊 Procedural Audio
- ✅ **6 BGM tracks** — all synthesized at runtime
- ✅ Chain x10 BGM dip + restore cycle
- ✅ Boss rage BGM volume swell
- ✅ Pitch-ramp SFX for jump, dash, kill, coin
- ✅ Stage-win fanfare + death sweep
- ✅ Zero audio files — 100% oscillator synthesis

</td>
</tr>
<tr>
<td width="50%">

### 🖥️ Interface
- ✅ Neon/synthwave palette — deep purple + cyan
- ✅ Parallax grid background + synthwave horizon
- ✅ Fake bloom (layered draw at low alpha)
- ✅ Boss health bar with phase indicators
- ✅ Boss WARNING banner on level 8 entry
- ✅ First-run tutorial: keycap symbol prompts

</td>
<td width="50%">

### ⚙️ Settings
- ✅ Master / Music / SFX volume sliders
- ✅ Fullscreen toggle
- ✅ Full key rebinding (Left / Right / Jump / Dash)
- ✅ Gamepad support (partial)
- ✅ Save file versioning — survives game updates
- ✅ Letterboxed virtual resolution (800×450)

</td>
</tr>
</table>

---

## 🕹️ Controls

<div align="center">

```
  [ ← ] [ → ]   —   Move left / right
  [ SPACE ]      —   Jump  (hold for higher jump)
  [ SHIFT ]      —   Dash  (8-directional)
  [ ESC ]        —   Pause / Resume
  [ R ]          —   Restart  (Game Over screen)
  [ F11 ]        —   Fullscreen
  [ F2 ]         —   Toggle Level Editor

  All keys are rebindable in Settings.
```

</div>

---

## 👾 Enemy Types

<div align="center">

| Enemy | Behaviour | Stage Design |
|:-----:|-----------|--------------|
| 🔴 **Chaser** | Tracks player horizontally | S1 circle → S2 crystal octagon → S3 lava triangle |
| 💎 **Spike** | Static / floating diamond | S1 diamond → S2 spinning hexagon → S3 fire-star |
| 🔫 **Shooter** | Fires projectiles at player | S1 hexagon+barrel → S2 diamond+crystal tip → S3 lava ember |
| 🦘 **Jumper** | Bounces rhythmically | S1 rounded rect → S2 hexagon+shards → S3 triangle+lava drips |

Each enemy type has unique silhouettes per stage — visually distinct at a glance.

</div>

---

## 🏆 Boss Roster

<div align="center">

| Boss | Stage | Phases | Mechanic |
|:----:|:-----:|:------:|----------|
| ⚔️ **Warden** | 1 | 2 | Speed surge + double shot on Phase 2 |
| 🔷 **Sentinel** | 2 | 3 | Progressive speed + triple shot on Phase 3 |
| 🔥 **Inferno** | 3 | 2 + Rage | Rage mode: BGM swell, max speed, rapid fire |

Phase transitions trigger screen shake, popup banners, and boss color shifts.

</div>

---

## 🛠️ Tech Stack

<div align="center">

| Layer | Technology | Purpose |
|-------|-----------|---------|
| Language | **C++17** | Core game logic, physics, rendering |
| Framework | **raylib 5.5** | Window, input, 2D drawing, audio device |
| Audio | **raylib AudioDevice + oscillators** | 100% procedural SFX + BGM synthesis |
| Physics | **Fixed timestep 60Hz** | Deterministic simulation, interpolated render |
| Web Build | **Emscripten 3.1.64** | Compiles C++ → WebAssembly |
| Build System | **CMake 3.15+** | Cross-platform (Windows MSVC + Web) |
| Hosting | **GitHub Pages** | Free static hosting |
| CI/CD | **GitHub Actions** | Auto-build WASM + deploy on every push |

</div>

---

## 🚀 Quick Start

### Play instantly (no setup)
```
https://mabdullahab614-alt.github.io/surge/
```

### Build on Windows (MSVC)
```bat
REM Requirements: Visual Studio 2022, CMake, Ninja
git clone https://github.com/mabdullahab614-alt/surge.git
cd surge

REM Run the build script (auto-finds vcvars64 + builds with Ninja)
_build.bat

REM Executable: build\Platformer.exe
```

### Build for Web (Emscripten)
```bash
# Requirements: emsdk 3.1.64, CMake, raylib 5.5 built for web
git clone https://github.com/mabdullahab614-alt/surge.git
cd surge

# Build raylib for web first (see deploy.yml for exact steps)
emcmake cmake -B build-web \
  -DRAYLIB_INCLUDE=path/to/raylib/src \
  -DRAYLIB_LIB=path/to/libraylib.a
emmake make -C build-web

# Output: build-web/index.html — open in any browser
```

### Deploy your own fork
```bash
git clone https://github.com/YOUR_USERNAME/surge.git

# Enable GitHub Pages:
# Settings → Pages → Source → GitHub Actions
# Your live URL: https://YOUR_USERNAME.github.io/surge/
```

---

## 📁 File Structure

```
surge/
│
├── 📁 src/
│   ├── ⚙️  main.cpp              # Entry point, game loop, persistence
│   ├── 🎮 game.cpp / game.h     # Physics, collision, chain-dash logic
│   ├── 🎨 render.cpp / render.h # All drawing — HUD, enemies, effects, UI screens
│   ├── 🔊 audio.cpp / audio.h   # Procedural SFX + BGM synthesis
│   ├── 🗺️  levels.cpp / levels.h # 24 hand-designed levels (3 stages × 8)
│   ├── 🔀 procgen.cpp / procgen.h # Procedural level generator
│   ├── 🌐 web_shell.html        # Synthwave-styled WASM canvas shell
│   ├── 📐 types.h               # Structs: GameState, Enemy, Particle…
│   ├── 🌍 globals.h             # Single global GameState instance
│   ├── 🎛️  Tuning.h              # All magic numbers as named constexpr
│   └── 🔧 AudioManager.h       # RAII audio wrapper
│
├── 📋 CMakeLists.txt            # Dual build: Windows MSVC + Emscripten web
├── 🔨 _build.bat                # One-click Windows MSVC build
├── 🔨 build_game.bat            # Alternative Windows build script
├── 📋 CLAUDE.md                 # Master architecture brief
├── 📄 CURRENT_STATE.md          # Feature status + design notes
├── 🔧 .gitignore                # Excludes build/, raylib/, save files
│
└── 📁 .github/
    └── 📁 workflows/
        └── deploy.yml           # CI/CD — Emscripten build + Pages deploy
```

---

## 🗺️ Roadmap

- [ ] 🏆 Online leaderboard (global high scores via Supabase)
- [ ] 📱 Mobile touch controls (on-screen d-pad)
- [ ] 🗺️ Level editor export — save & share custom levels
- [ ] 🌍 Stage 4+ with new enemy types and biomes
- [ ] 🎨 Alternate player skins unlocked by chain records
- [ ] 💀 Daily challenge mode — fixed seed, one attempt

> Have an idea? [Open a Feature Request](https://github.com/mabdullahab614-alt/surge/issues/new)

---

## 🤝 Contributing

Contributions are welcome! The codebase is single-file-per-system and well-commented.

1. Fork the project
2. Create your branch: `git checkout -b feature/amazing-feature`
3. Commit: `git commit -m 'feat: add amazing feature'`
4. Push: `git push origin feature/amazing-feature`
5. Open a [Pull Request](https://github.com/mabdullahab614-alt/surge/pulls)

---

## 📜 License

Distributed under the **MIT License**.

```
Copyright (c) 2026 Abdullah Javid
Free to use, modify, and distribute with attribution.
```

---

## 👤 Author

<div align="center">

<img src="https://capsule-render.vercel.app/api?type=rect&color=0:1a0033,100:0d001a&height=130&text=Abdullah%20Javid&fontSize=38&fontColor=00f5ff&fontAlignY=45&stroke=00f5ff&strokeWidth=1&desc=Undergraduate%20AI%20Engineer%20%7C%20Game%20Developer&descSize=15&descAlignY=72&descColor=ff006e" width="100%"/>

<br/>

[![GitHub](https://img.shields.io/badge/GitHub-mabdullahab614--alt-00f5ff?style=for-the-badge&logo=github)](https://github.com/mabdullahab614-alt)
[![Email](https://img.shields.io/badge/Email-asoftwarer4.5%40gmail.com-ff2d2d?style=for-the-badge&logo=gmail&logoColor=white)](mailto:asoftwarer4.5@gmail.com)

*"Chain the speed. Break the level."*

</div>

---

<div align="center">

**If SURGE made your fingers fly, drop a ⭐ — it means the world.**

<img src="https://capsule-render.vercel.app/api?type=waving&color=0:0d001a,100:1a0033&height=120&section=footer&text=SURGE&fontSize=32&fontColor=00f5ff&animation=fadeIn&fontAlignY=65"/>

[![Play Now](https://img.shields.io/badge/⚡%20PLAY%20NOW-00f5ff?style=for-the-badge)](https://mabdullahab614-alt.github.io/surge/)

</div>
