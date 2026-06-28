# Retro-Style Match-3 Engine (Modern C++23 & Raylib)

A custom-built Match-3 game engine written in **Modern C++23** and powered by **Raylib** for high-performance 2D rendering and spatial audio. This project showcases software engineering fundamentals, including modular object-oriented design, deterministic state machines, mathematical coordinate mapping, and robust resource management.

---

## 🎮 Technical Overview

The application generates an interactive $8 \times 8$ grid of game tiles represented by distinct ASCII characters (`#`, `@`, `$`, `%`, `&`). Players select tiles using mouse coordinates and swap them with adjacent neighbors. The core engine detects linear combinations of three or more matching tiles, resolves them, triggers falling physics for above tiles, and spawns new ones in a cascading animation loop.

### 📸 Preview / Showcase

- **Graphics Pipeline:** Standardised rendering utilizing Raylib's texture mapping, alpha blending, and custom typography fonts.
- **Interactive UI:** Smooth selection outline highlighting, scaling text components, and floating pop-up score animations.
- **Audio Environment:** Asynchronous background music stream coupled with localized matching sound effects.

---

## 🚀 Key Architectural Features

- **Deterministic Finite State Machine (FSM):** The engine operates via three discrete game states (`TileState::Idle`, `TileState::Animating`, `TileState::MatchDelay`). This architecture decouples user inputs from resolution animations, preventing race conditions and input lag.
- **Dynamic Gravity & Fall Interpolation:** Implements localized vertical offset telemetry per cell, animating falling tiles with precise pixel-rate interpolation until they snap to their stable grid coordinates.
- **Dual-Sweep Match Logic:** Utilizes a double-sweep scan (horizontal and vertical) across the structural matrix to identify linear chains of length $\ge 3$. Matches are validated onto a temporary boolean map to correctly resolve intersecting structures (L-shapes, T-shapes) in a single frame.
- **Resource Management (RAII):** Strictly implements RAII (Resource Acquisition Is Initialization) paradigms for lifecycle management of GPU texture structures, audio device streams, sound buffers, and file decoders, ensuring zero memory leaks on shutdown.
- **Modern C++ Standard:** Employs C++23 features, clean namespaces, modern RNG setups (`std::mt19937` with standard distributions), and standard container manipulation (`std::array`, `std::vector`, `std::erase_if`).

---

## 📁 Directory Structure

```text
├── assets/
│   ├── background.png    # High-resolution background artwork
│   ├── bgm.mp3           # Asymmetric loops for background audio
│   ├── match.mp3         # Match confirmation sound effect
│   └── score-font.ttf    # Custom score indicator TTF font
├── main.cpp              # Primary application logic and Game controller
└── README.md             # Project documentation (this file)
```

---

## 🛠️ Build and Compilation Instructions

### Prerequisites

To build the engine, you must install a modern C++ compiler supporting C++23 (`g++ >= 13` or `clang >= 16`) and the **Raylib** library.

#### Install Raylib:

- **Debian/Ubuntu**:
  ```bash
  sudo apt-get install libraylib-dev
  ```
- **macOS (Homebrew)**:
  ```bash
  brew install raylib
  ```
- **Windows**: Install using MinGW or MSVC according to the [Raylib Windows Guide](https://github.com/raysan5/raylib/wiki/Working-on-Windows).

---

### Compilation Commands

Compile using the standard C++23 flag and link the Raylib library.

#### Linux & macOS:

```bash
g++ -std=c++23 main.cpp -o match3 -lraylib
./match3
```

#### Windows (GCC/MinGW):

```bash
g++ -std=c++23 main.cpp -o match3.exe -lraylib -lopengl32 -lgdi32 -lwinmm
./match3.exe
```

---

## 🧠 Core Engineering Design Decisions

### 1. Input Isolation

Inputs are filtered through state checks. When tiles are falling or waiting to cascade (`TileState::Animating` or `TileState::MatchDelay`), inputs are locked. This guarantees that user interactions cannot break grid array states while gravity calculations are running.

### 2. Multi-Match Resolution

The resolution sweep flags cells to be cleared on a secondary boolean matrix `matched_` instead of editing the board on the fly. This prevents premature modifications of tiles from interfering with subsequent direction checks on the same step.
