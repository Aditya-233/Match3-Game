# Ultra-Lean Match-3 Engine (Modern C++23 & Raylib)

A high-performance, ultra-lean Match-3 puzzle game engine written in **Modern C++23** and powered by **Raylib**. Designed with a hyper-compact single-translation-unit architecture (~100 lines of code), zero over-engineering, robust resource management, and full support for dynamic window resizing.

---

## 🎮 Technical Overview

The application manages an interactive $8 \times 8$ grid of gem tiles utilizing custom HD sprite textures (Ruby, Sapphire, Emerald, Amethyst, Amber). Players select tiles using mouse input and swap adjacent neighbors. The engine detects 3-in-a-row linear combinations across horizontal and vertical axes, awards points (+10 per tile), clears matched cells, reverts invalid swaps, and triggers cascading gravity drops until the board stabilizes.

---

## 🚀 Key Architectural Features

- **Hyper-Lean Codebase (~100 LOC):** Clean, zero-bloat C++23 implementation prioritizing performance, high code density, and maintainability.
- **Dynamic Window Resizing:** Dynamically recalculates viewport bounds (`boardSize`), cell width (`cs`), and grid offsets (`ox`, `oy`) every frame to center the board on any window resolution or display scale.
- **Smart Non-Matching Board Initializer:** Generates an initial grid with zero pre-existing 3-in-a-row matches in a single pass without retry loops.
- **Cascading Gravity & Move Validation:** Reverts illegal swaps that do not form a match, and cascades downward tile drops iteratively until all chain reactions resolve.
- **Strict Compiler Diagnostics:** Compiles cleanly with **0 warnings and 0 errors** under extreme GCC warning flags (`-Werror`, `-Wall`, `-Wextra`, `-Wpedantic`, `-Wconversion`, `-Wshadow`, `-Wold-style-cast`).
- **RAII Resource Management:** Safe initialization and explicit unloading (`UnloadTexture`, `UnloadFont`, `UnloadSound`, `UnloadMusicStream`) for GPU textures, audio streams, and custom fonts on shutdown.

---

## 📁 Project Directory Structure

```text
Match3-Game/
├── assets/
│   ├── background.png   # High-resolution cosmic backdrop image
│   ├── bgm.mp3          # Looping background music track
│   ├── font.ttf         # Futuristic Orbitron-Bold TTF font
│   ├── gem_0.png        # Ruby Red Diamond gem texture
│   ├── gem_1.png        # Sapphire Blue Sphere gem texture
│   ├── gem_2.png        # Emerald Green Square gem texture
│   ├── gem_3.png        # Amethyst Purple Hexagon gem texture
│   ├── gem_4.png        # Amber Orange Triangle gem texture
│   ├── match.wav        # Amplified tile match SFX (+9.5 dB)
│   └── swap.wav         # Tile swap SFX
├── main.cpp             # Primary application logic and game controller
└── README.md            # Project documentation
```

---

## 🛠️ Build and Compilation Instructions

### Prerequisites

You need a modern C++ compiler supporting C++23 (`g++ >= 13` or `clang >= 16`) and the **Raylib** development headers.

#### Install Raylib:

- **Arch Linux**:
  ```bash
  sudo pacman -S raylib
  ```
- **Debian / Ubuntu**:
  ```bash
  sudo apt-get install libraylib-dev
  ```
- **macOS (Homebrew)**:
  ```bash
  brew install raylib
  ```

---

### Compilation Commands

#### Standard Build:

```bash
g++ -std=c++23 main.cpp -o match3 -lraylib
./match3
```

#### Strict Diagnostic Build (Recommended):

```bash
g++ -std=c++23 main.cpp -o match3 -lraylib \
  -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion \
  -Wshadow -Wformat=2 -Wunused -Wcast-align -Wdouble-promotion \
  -Wnon-virtual-dtor -Wnull-dereference -Wlogical-op -Wundef \
  -Wcast-qual -Wold-style-cast -Woverloaded-virtual -Wctor-dtor-privacy
./match3
```

---

## 🧠 Core Engineering Principles

1. **Dual-Direction Scan Loop:** Scans horizontal (`i, j..j+2`) and vertical (`j..j+2, i`) matrix lines in a single pass onto a boolean matrix before clearing, preserving chain reaction integrity.
2. **Dynamic Viewport Scaling:** Window layout relies strictly on relative runtime queries (`GetScreenWidth()`, `GetScreenHeight()`) rather than hardcoded positions, ensuring responsive scaling across screen sizes.
3. **Balanced Audio Engineering:** Master gain levels are balanced (`bgm = 0.3`, `swap = 0.35`, `match = 1.0`) so sound effects cut through clearly over the background music stream.
