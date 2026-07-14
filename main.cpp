// Compile with: g++ -std=c++23 main.cpp -o match3 -lraylib -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -Wshadow -Wformat=2 -Wunused -Wcast-align -Wdouble-promotion -Wnon-virtual-dtor -Wnull-dereference -Wlogical-op -Wundef -Wcast-qual -Wold-style-cast -Woverloaded-virtual -Wctor-dtor-privacy

#include <algorithm>

#include "raylib.h"

constexpr int G = 8;

int main(void) {
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(480, 520, "Match-3");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    InitAudioDevice();

    Texture2D bg = LoadTexture("assets/background.png");
    Texture2D gems[5];
    for (int i = 0; i < 5; i++) {
        gems[i] = LoadTexture(TextFormat("assets/gem_%d.png", i));
    }
    Font font = LoadFontEx("assets/font.ttf", 48, nullptr, 0);
    Music bgm = LoadMusicStream("assets/bgm.mp3");
    Sound swapSnd = LoadSound("assets/swap.wav");
    Sound matchSnd = LoadSound("assets/match.wav");

    if (bgm.stream.buffer != nullptr) {
        PlayMusicStream(bgm);
        SetMusicVolume(bgm, 0.3f);
    }
    if (swapSnd.stream.buffer != nullptr) {
        SetSoundVolume(swapSnd, 0.35f);
    }
    if (matchSnd.stream.buffer != nullptr) {
        SetSoundVolume(matchSnd, 1.0f);
    }

    int board[G][G], score = 0, selR = -1, selC = -1;

    for (int r = 0; r < G; r++)
        for (int c = 0; c < G; c++)
            do {
                board[r][c] = rand() % 5;
            } while ((c >= 2 && board[r][c - 1] == board[r][c] && board[r][c - 2] == board[r][c]) || (r >= 2 && board[r - 1][c] == board[r][c] && board[r - 2][c] == board[r][c]));

    while (!WindowShouldClose()) {
        if (bgm.stream.buffer != nullptr) UpdateMusicStream(bgm);

        int sw = GetScreenWidth(), sh = GetScreenHeight();
        int boardSize = static_cast<int>(static_cast<float>(std::min(sw, sh)) * 0.75f);
        int cs = boardSize / G, ox = (sw - boardSize) / 2, oy = (sh - boardSize) / 2;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int mx = GetMouseX() - ox, my = GetMouseY() - oy, mc = mx / cs, mr = my / cs;
            if (mx >= 0 && my >= 0 && mc < G && mr < G) {
                if (selR < 0) {
                    selR = mr;
                    selC = mc;
                } else {
                    if (std::abs(mc - selC) + std::abs(mr - selR) == 1) {
                        std::swap(board[selR][selC], board[mr][mc]);
                        bool matchFound;
                        int pass = 0;
                        do {
                            bool m[G][G] = {};
                            matchFound = false;

                            for (int i = 0; i < G; i++)
                                for (int j = 0; j < G - 2; j++) {
                                    if (board[i][j] >= 0 && board[i][j] == board[i][j + 1] && board[i][j] == board[i][j + 2]) {
                                        m[i][j] = m[i][j + 1] = m[i][j + 2] = matchFound = true;
                                    }
                                    if (board[j][i] >= 0 && board[j][i] == board[j + 1][i] && board[j][i] == board[j + 2][i]) {
                                        m[j][i] = m[j + 1][i] = m[j + 2][i] = matchFound = true;
                                    }
                                }

                            if (matchFound) {
                                for (int r = 0; r < G; r++)
                                    for (int c = 0; c < G; c++) {
                                        if (m[r][c]) {
                                            board[r][c] = -1;
                                            score += 10;
                                        }
                                    }

                                for (int c = 0; c < G; c++) {
                                    int w = G - 1;
                                    for (int r = G - 1; r >= 0; r--) {
                                        if (board[r][c] >= 0) {
                                            board[w--][c] = board[r][c];
                                        }
                                    }

                                    for (int r = w; r >= 0; r--) {
                                        board[r][c] = rand() % 5;
                                    }
                                }

                                if (matchSnd.stream.buffer != nullptr) {
                                    PlaySound(matchSnd);
                                }
                                pass++;
                            }
                        } while (matchFound);
                        if (pass == 0) {
                            std::swap(board[selR][selC], board[mr][mc]);

                            if (swapSnd.stream.buffer != nullptr) {
                                PlaySound(swapSnd);
                            }
                        }
                        selR = selC = -1;
                    } else {
                        selR = mr;
                        selC = mc;
                    }
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (bg.id > 0) {
            DrawTexturePro(bg, Rectangle{0.0f, 0.0f, static_cast<float>(bg.width), static_cast<float>(bg.height)}, Rectangle{0.0f, 0.0f, static_cast<float>(sw), static_cast<float>(sh)}, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
        } else {
            DrawRectangleGradientV(0, 0, sw, sh, Color{44, 62, 80, 255}, BLACK);
        }

        DrawRectangleRounded(Rectangle{static_cast<float>(ox), static_cast<float>(oy), static_cast<float>(boardSize), static_cast<float>(boardSize)}, 0.03f, 4, Color{20, 20, 25, 180});
        DrawRectangleRoundedLinesEx(Rectangle{static_cast<float>(ox), static_cast<float>(oy), static_cast<float>(boardSize), static_cast<float>(boardSize)}, 0.03f, 4, 3.0f, Color{255, 255, 255, 45});

        for (int r = 0; r < G; r++)
            for (int c = 0; c < G; c++) {
                DrawRectangleLines(ox + c * cs, oy + r * cs, cs, cs, Color{255, 255, 255, 12});

                int t = board[r][c];
                if (t >= 0 && t < 5) {
                    if (gems[t].id > 0) {
                        Rectangle srcRec{0.0f, 0.0f, static_cast<float>(gems[t].width), static_cast<float>(gems[t].height)};
                        Rectangle destRec{static_cast<float>(ox + c * cs + 2), static_cast<float>(oy + r * cs + 2), static_cast<float>(cs - 4), static_cast<float>(cs - 4)};
                        DrawTexturePro(gems[t], srcRec, destRec, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                    }
                }
            }

        if (selR >= 0 && selC >= 0) {
            DrawRectangleRoundedLinesEx(Rectangle{static_cast<float>(ox + selC * cs + 2), static_cast<float>(oy + selR * cs + 2), static_cast<float>(cs - 4), static_cast<float>(cs - 4)}, 0.15f, 4, 3.0f, Color{241, 196, 15, 255});
        }

        const char* scoreText = TextFormat("SCORE: %d", score);
        if (font.texture.id > 0) {
            Vector2 scoreSize = MeasureTextEx(font, scoreText, 28, 2);
            DrawTextEx(font, scoreText, Vector2{static_cast<float>(sw - static_cast<int>(scoreSize.x)) / 2.0f, static_cast<float>(oy - static_cast<int>(scoreSize.y) - 12)}, 28, 2, WHITE);
        } else {
            int scoreWidth = MeasureText(scoreText, 20);
            DrawText(scoreText, (sw - scoreWidth) / 2, oy - 28, 20, WHITE);
        }

        EndDrawing();
    }

    for (int i = 0; i < 5; i++) {
        if (gems[i].id > 0) {
            UnloadTexture(gems[i]);
        }
    }
    if (bg.id > 0) {
        UnloadTexture(bg);
    }
    if (font.texture.id > 0) {
        UnloadFont(font);
    }
    if (bgm.stream.buffer != nullptr) {
        UnloadMusicStream(bgm);
    }
    if (swapSnd.stream.buffer != nullptr) {
        UnloadSound(swapSnd);
    }
    if (matchSnd.stream.buffer != nullptr) {
        UnloadSound(matchSnd);
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
