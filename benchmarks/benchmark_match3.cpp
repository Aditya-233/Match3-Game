// Match3-Game Raylib In-Loop Graphical Benchmark
//
// HOW TO EXECUTE:
//      Compile & execute manually:
//      g++ -std=c++23 benchmarks/benchmark_match3.cpp -o benchmarks/bench_bin -lraylib -O3 && ./benchmarks/bench_bin

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <vector>

#include "raylib.h"

constexpr int G = 8;

int main(void) {
    // Enable hidden window mode to support headless benchmarking while preserving window & render contexts
    SetConfigFlags(FLAG_WINDOW_HIDDEN | FLAG_WINDOW_HIGHDPI);
    InitWindow(480, 520, "Match3 Raylib In-Loop Benchmark");
    SetTargetFPS(60);
    InitAudioDevice();

    int board[G][G];
    int score = 0;

    // Board initialization loop
    for (int r = 0; r < G; r++) {
        for (int c = 0; c < G; c++) {
            do {
                board[r][c] = rand() % 5;
            } while ((c >= 2 && board[r][c - 1] == board[r][c] && board[r][c - 2] == board[r][c]) ||
                     (r >= 2 && board[r - 1][c] == board[r][c] && board[r - 2][c] == board[r][c]));
        }
    }

    constexpr int TARGET_CASCADES = 100;
    int cascade_events_recorded = 0;
    int total_frames = 0;

    std::vector<float> frame_delta_times_ms;
    std::vector<double> cascade_step_times_us;
    frame_delta_times_ms.reserve(1000);
    cascade_step_times_us.reserve(500);

    std::cout << "=========================================================\n";
    std::cout << "     MATCH3 RAYLIB IN-LOOP GRAPHICAL BENCHMARK HARNESS   \n";
    std::cout << "=========================================================\n";
    std::cout << "[*] Executing within active Raylib window loop...\n";

    // Main game render loop matching main.cpp structure exactly
    while (!WindowShouldClose() && cascade_events_recorded < TARGET_CASCADES) {
        total_frames++;

        // Programmatically inject swap commands to force cascades inside the loop
        int r1 = rand() % G, c1 = rand() % G;
        int r2 = r1, c2 = (c1 < G - 1) ? c1 + 1 : c1 - 1;

        std::swap(board[r1][c1], board[r2][c2]);

        auto cascade_start = std::chrono::high_resolution_clock::now();
        bool matchFound;
        int pass = 0;
        do {
            bool m[G][G] = {};
            matchFound = false;
            for (int i = 0; i < G; i++) {
                for (int j = 0; j < G - 2; j++) {
                    if (board[i][j] >= 0 && board[i][j] == board[i][j + 1] && board[i][j] == board[i][j + 2])
                        m[i][j] = m[i][j + 1] = m[i][j + 2] = matchFound = true;
                    if (board[j][i] >= 0 && board[j][i] == board[j + 1][i] && board[j][i] == board[j + 2][i])
                        m[j][i] = m[j + 1][i] = m[j + 2][i] = matchFound = true;
                }
            }
            if (matchFound) {
                for (int r = 0; r < G; r++) {
                    for (int c = 0; c < G; c++) {
                        if (m[r][c]) {
                            board[r][c] = -1;
                            score += 10;
                        }
                    }
                }
                for (int c = 0; c < G; c++) {
                    int w = G - 1;
                    for (int r = G - 1; r >= 0; r--)
                        if (board[r][c] >= 0) board[w--][c] = board[r][c];
                    for (int r = w; r >= 0; r--) board[r][c] = rand() % 5;
                }
                pass++;
            }
        } while (matchFound);

        auto cascade_end = std::chrono::high_resolution_clock::now();

        if (pass == 0) {
            std::swap(board[r1][c1], board[r2][c2]);  // Revert swap
        } else {
            cascade_events_recorded += pass;
            double elapsed_us = std::chrono::duration<double, std::micro>(cascade_end - cascade_start).count();
            cascade_step_times_us.push_back(elapsed_us);
        }

        // Layout Math preserved explicitly from main.cpp
        int sw = GetScreenWidth(), sh = GetScreenHeight();
        int boardSize = static_cast<int>(static_cast<float>(std::min(sw, sh)) * 0.75f);
        int cs = boardSize / G, ox = (sw - boardSize) / 2, oy = (sh - boardSize) / 2;

        // Render pass execution matching main.cpp pipeline
        BeginDrawing();
        ClearBackground(BLACK);

        DrawRectangleGradientV(0, 0, sw, sh, Color{44, 62, 80, 255}, BLACK);
        DrawRectangleRounded(Rectangle{static_cast<float>(ox) - 8.0f, static_cast<float>(oy) - 8.0f, static_cast<float>(boardSize) + 16.0f, static_cast<float>(boardSize) + 16.0f}, 0.03f, 4, Color{20, 20, 25, 180});
        DrawRectangleRoundedLinesEx(Rectangle{static_cast<float>(ox) - 9.0f, static_cast<float>(oy) - 9.0f, static_cast<float>(boardSize) + 18.0f, static_cast<float>(boardSize) + 18.0f}, 0.03f, 4, 3.0f, Color{255, 255, 255, 45});

        for (int r = 0; r < G; r++) {
            for (int c = 0; c < G; c++) {
                DrawRectangleLines(ox + c * cs, oy + r * cs, cs, cs, Color{255, 255, 255, 12});
            }
        }

        // Text formatting and measurement overhead inside render loop
        const char* scoreText = TextFormat("SCORE: %d", score);
        int scoreWidth = MeasureText(scoreText, 20);
        DrawText(scoreText, (sw - scoreWidth) / 2, oy - 28, 20, WHITE);

        EndDrawing();

        // Capture exact frame delta time (GetFrameTime returns seconds as float)
        float frame_delta_ms = GetFrameTime() * 1000.0f;
        frame_delta_times_ms.push_back(frame_delta_ms);
    }

    CloseAudioDevice();
    CloseWindow();

    std::sort(frame_delta_times_ms.begin(), frame_delta_times_ms.end());
    std::sort(cascade_step_times_us.begin(), cascade_step_times_us.end());

    float frame_p50 = frame_delta_times_ms[frame_delta_times_ms.size() * 0.50];
    float frame_p95 = frame_delta_times_ms[frame_delta_times_ms.size() * 0.95];
    float frame_p99 = frame_delta_times_ms[frame_delta_times_ms.size() * 0.99];

    double cascade_p50 = cascade_step_times_us.empty() ? 0.0 : cascade_step_times_us[cascade_step_times_us.size() * 0.50];
    double cascade_p95 = cascade_step_times_us.empty() ? 0.0 : cascade_step_times_us[cascade_step_times_us.size() * 0.95];

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "[+] Total Frames Evaluated:       " << total_frames << "\n";
    std::cout << "[+] Total Cascade Passes Forced:  " << cascade_events_recorded << "\n";
    std::cout << "[+] Frame Delta Time P50:         " << frame_p50 << " ms\n";
    std::cout << "[+] Frame Delta Time P95:         " << frame_p95 << " ms\n";
    std::cout << "[+] Frame Delta Time P99:         " << frame_p99 << " ms\n";
    std::cout << "[+] In-Loop Cascade Step P50:     " << cascade_p50 << " us\n";
    std::cout << "[+] In-Loop Cascade Step P95:     " << cascade_p95 << " us\n";

    return 0;
}
