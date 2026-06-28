#include "raylib.h"
#include <array>
#include <random>

namespace MatchGame {
namespace {

/**
 * @brief Represents the internal state machine for managing grid transitions.
 */
enum class TileState : std::uint8_t {
    Idle,      // Board is stable; awaiting player input.
    Animating, // Board is resolving gravity; shifting elements downward.
    MatchDelay // Brief temporal padding before testing cascading evaluations.
};

/**
 * @brief Visual overlay structure displaying scored values floating over cleared coordinate paths.
 */
struct ScorePopup {
    Vector2 position{};
    int amount{0};
    float lifetime{1.0F};
    float alpha{1.0F};
};

/**
 * @brief Complete controller managing match-3 board telemetry, input routing, scoring logic, and drawing sequences.
 */
class Game {
  public:
    /**
     * @brief Context lifetime manager initializing the graphics frame context, operational limits, audio context, and background memory allocations.
     * @param window_width Demanded screen viewport layout span.
     * @param window_height Demanded screen viewport layout elevation.
     * @param title Desktop decoration identifier string.
     */
    Game(int window_width, int window_height, const char *title) {
        InitWindow(window_width, window_height, title);
        SetTargetFPS(60); // Standard hardware refresh target rate
        InitAudioDevice();

        background_ = LoadTexture("assets/background.png");
        score_font_ = LoadFontEx("assets/score-font.ttf", 32, nullptr, 0); // 32pt allocation size
        background_music_ = LoadMusicStream("assets/bgm.mp3");
        match_sound_ = LoadSound("assets/match.mp3");

        PlayMusicStream(background_music_);
        SetMusicVolume(background_music_, 0.60F); // 60% master music level
        SetSoundVolume(match_sound_, 0.40F);      // 40% master sound effect level

        InitBoard();
    }

    /**
     * @brief Safe resource cleanup pipeline offloading operational asset registers from active runtime environments.
     */
    ~Game() {
        StopMusicStream(background_music_);
        UnloadMusicStream(background_music_);
        UnloadSound(match_sound_);
        UnloadFont(score_font_);
        UnloadTexture(background_);
        CloseAudioDevice();
        CloseWindow();
    }

    Game(const Game &) = delete;
    Game &operator=(const Game &) = delete;
    Game(Game &&) = delete;
    Game &operator=(Game &&) = delete;

    /**
     * @brief Drives the application process frame loop until intercepting application termination triggers.
     */
    void Run() {
        while (!WindowShouldClose()) {
            Update();
            Draw();
        }
    }

  private:
    std::vector<ScorePopup> score_popups_;
    Font score_font_{};
    Music background_music_{};
    Sound match_sound_{};
    Texture2D background_{};

    std::array<std::array<float, 8>, 8> fall_offset_{}; // Fixed 8x8 structural grid size tracking vertical shifts
    Vector2 grid_origin_{.x = 0.0F, .y = 0.0F};
    Vector2 selected_tile_{.x = -1.0F, .y = -1.0F};
    float match_delay_timer_{0.0F};
    float score_scale_{1.0F};
    float score_scale_velocity_{0.0F};
    int score_{0};

    std::array<std::array<char, 8>, 8> board_{};   // Fixed 8x8 layout character data matrix
    std::array<std::array<bool, 8>, 8> matched_{}; // Fixed 8x8 layout match validation map
    TileState tile_state_{TileState::Idle};
    bool score_animating_{false};

    /**
     * @brief Generates pseudo-random asset entries selecting from uniform sets containing structural definitions {'#', '@', '$', '%', '&'}.
     * @return Single character designation acting as a discrete grid object identifier.
     */
    static char GetRandomTile() {
        static std::mt19937 rng{std::random_device{}()};
        static std::uniform_int_distribution<std::size_t> dist(0, 4); // 0 to 4 indexes 5 unique structural variants
        static const std::array<char, 5> types = {'#', '@', '$', '%', '&'};
        return types.at(dist(rng));
    }

    /**
     * @brief Switches positional coordinates of two discrete elements mapped onto the tracking coordinate arrays.
     */
    void SwapTiles(std::size_t col1, std::size_t row1, std::size_t col2, std::size_t row2) {
        std::swap(board_.at(row1).at(col1), board_.at(row2).at(col2));
    }

    /**
     * @brief Evaluates spatial Manhattan Distance metrics to confirm if target structures sit directly orthogonal adjacent.
     * @return Returns true only when horizontal distance plus vertical distance equals exactly one unit space.
     */
    static bool AreTilesAdjacent(Vector2 pos_a, Vector2 pos_b) {
        // Orthogonal neighbor step verify
        return (std::abs(static_cast<int>(pos_a.x) - static_cast<int>(pos_b.x)) + std::abs(static_cast<int>(pos_a.y) - static_cast<int>(pos_b.y))) == 1;
    }

    /**
     * @brief Spawns visual text indicators centered near the coordinate origins of cleared board positions.
     */
    void AddScorePopup(std::size_t col, std::size_t row, int amount) {
        // Total 1.0 second duration limit
        // 42.0F layout cell dimension; 2.0F center divisor
        score_popups_.emplace_back(ScorePopup{.position = Vector2{.x = grid_origin_.x + (static_cast<float>(col) * 42.0F) + (42.0F / 2.0F), .y = grid_origin_.y + (static_cast<float>(row) * 42.0F) + (42.0F / 2.0F)}, .amount = amount, .lifetime = 1.0F, .alpha = 1.0F});
    }

    /**
     * @brief Modifies overall context metrics, activates audio alerts, and triggers interface scaling transformations upon successful matches.
     */
    void TriggerScoreSequence(std::size_t col, std::size_t row) {
        score_ += 10; // Incremental point rewards per single asset removal
        PlaySound(match_sound_);
        score_animating_ = true;
        score_scale_ = 2.0F;           // Text expands to 200% scale instantly
        score_scale_velocity_ = -2.5F; // Shrink speed factor back to rest state
        AddScorePopup(col, row, 10);
    }

    /**
     * @brief Scans horizontal and vertical dimensional paths across the 8x8 matrix tracking matching contiguous rows of size three or greater.
     * @return True if at least one multi-element match cluster is flagged.
     */
    bool FindMatches() {
        auto found = false;

        for (auto &row_arr : matched_) {
            row_arr.fill(false);
        }

        // Horizontal checking iteration space limits: 8 rows, up to index 5 (8 - 2 padding space) to verify triads
        for (auto row = static_cast<std::size_t>(0); row < 8; row++) {
            for (auto col = static_cast<std::size_t>(0); col < 6; col++) {
                auto current = board_.at(row).at(col);
                if (current != ' ' && current == board_.at(row).at(col + 1) && current == board_.at(row).at(col + 2)) {
                    matched_.at(row).at(col) = true;
                    matched_.at(row).at(col + 1) = true;
                    matched_.at(row).at(col + 2) = true;
                    TriggerScoreSequence(col, row);
                    found = true;
                }
            }
        }

        // Vertical checking iteration space limits: 8 columns, up to index 5 (8 - 2 padding space) to verify triads
        for (auto col = static_cast<std::size_t>(0); col < 8; col++) {
            for (auto row = static_cast<std::size_t>(0); row < 6; row++) {
                auto current = board_.at(row).at(col);
                if (current != ' ' && current == board_.at(row + 1).at(col) && current == board_.at(row + 2).at(col)) {
                    matched_.at(row).at(col) = true;
                    matched_.at(row + 1).at(col) = true;
                    matched_.at(row + 2).at(col) = true;
                    TriggerScoreSequence(col, row);
                    found = true;
                }
            }
        }

        return found;
    }

    /**
     * @brief Iterates backwards across columns to apply gravity constraints; clear spaces collapse upward as new data populates top slots.
     */
    void ResolveMatches() {
        for (auto col = static_cast<std::size_t>(0); col < 8; col++) {
            auto write_row = 7; // Bottom index row floor limit for 8-sized matrix bounds

            for (auto row = 7; row >= 0; row--) {
                auto u_row = static_cast<std::size_t>(row);
                auto u_write = static_cast<std::size_t>(write_row);

                if (!matched_.at(u_row).at(col)) {
                    if (row != write_row) {
                        board_.at(u_write).at(col) = board_.at(u_row).at(col);
                        fall_offset_.at(u_write).at(col) = static_cast<float>(write_row - row) * 42.0F; // 42.0F scale dimension transformation step
                        board_.at(u_row).at(col) = ' ';
                    }
                    write_row--;
                }
            }

            while (write_row >= 0) {
                auto u_write = static_cast<std::size_t>(write_row);
                board_.at(u_write).at(col) = GetRandomTile();
                fall_offset_.at(u_write).at(col) = static_cast<float>(write_row + 1) * 42.0F; // Generates dynamic fall heights outside top boundaries
                write_row--;
            }
        }

        tile_state_ = TileState::Animating;
    }

    /**
     * @brief Allocates elements to build initial boards, positions the layout base, and verifies clear fields free of starting chains.
     */
    void InitBoard() {
        for (auto &row_arr : board_) {
            for (auto &tile : row_arr) {
                tile = GetRandomTile();
            }
        }

        // Center calculation mapping offsets: Total structure size is 336.0F (8 rows/cols * 42.0F dimension) split via half width scaling
        grid_origin_ = Vector2{.x = (static_cast<float>(GetScreenWidth()) - 336.0F) / 2.0F, .y = (static_cast<float>(GetScreenHeight()) - 336.0F) / 2.0F};

        if (FindMatches()) {
            ResolveMatches();
        } else {
            tile_state_ = TileState::Idle;
        }
    }

    /**
     * @brief Normalizes raw mouse pointer screenspace offsets into local indexing arrays to track context swaps.
     * @param mouse Current multi-axis coordinates reported by input polling.
     */
    void HandleInput(Vector2 mouse) {
        if (tile_state_ != TileState::Idle || !IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            return;
        }

        auto grid_rect = Rectangle{
            .x = grid_origin_.x,
            .y = grid_origin_.y,
            .width = 336.0F,   // 8 structural columns * 42.0F pixel width mapping dimension
            .height = 336.0F}; // 8 structural rows * 42.0F pixel height mapping dimension

        if (CheckCollisionPointRec(mouse, grid_rect)) {
            auto raw_col = static_cast<int>((mouse.x - grid_origin_.x) / 42.0F);
            auto raw_row = static_cast<int>((mouse.y - grid_origin_.y) / 42.0F);

            auto col = static_cast<std::size_t>(raw_col);
            auto row = static_cast<std::size_t>(raw_row);
            auto current_tile = Vector2{.x = static_cast<float>(col), .y = static_cast<float>(row)};

            if (selected_tile_.x < 0.0F) {
                selected_tile_ = current_tile;
            } else {
                if (AreTilesAdjacent(selected_tile_, current_tile)) {
                    auto sel_col = static_cast<std::size_t>(selected_tile_.x);
                    auto sel_row = static_cast<std::size_t>(selected_tile_.y);
                    SwapTiles(sel_col, sel_row, col, row);

                    if (FindMatches()) {
                        ResolveMatches();
                    } else {
                        SwapTiles(sel_col, sel_row, col, row);
                    }
                }
                selected_tile_ = Vector2{.x = -1.0F, .y = -1.0F}; // Null setting configuration mapping flags
            }
        }
    }

    /**
     * @brief Interpolates current tile offsets to simulate descending layout gravity steps across frames.
     */
    void ProcessFallAnimations() {
        if (tile_state_ != TileState::Animating) {
            return;
        }

        auto still_animating = false;
        for (auto row = static_cast<std::size_t>(0); row < 8; row++) {
            for (auto col = static_cast<std::size_t>(0); col < 8; col++) {
                if (fall_offset_.at(row).at(col) > 0.0F) {
                    fall_offset_.at(row).at(col) -= 8.0F; // Constant frame-linear translation pixel dropping offset acceleration step
                    if (fall_offset_.at(row).at(col) < 0.0F) {
                        fall_offset_.at(row).at(col) = 0.0F;
                    } else {
                        still_animating = true;
                    }
                }
            }
        }

        if (!still_animating) {
            tile_state_ = TileState::MatchDelay;
            match_delay_timer_ = 0.2F; // 0.2s pause structural delay threshold hold
        }
    }

    /**
     * @brief Evaluates temporal gaps using system frame updates to accurately pause cascading board operations.
     */
    void ProcessMatchDelay() {
        if (tile_state_ != TileState::MatchDelay) {
            return;
        }

        match_delay_timer_ -= GetFrameTime();
        if (match_delay_timer_ <= 0.0F) {
            if (FindMatches()) {
                ResolveMatches();
            } else {
                tile_state_ = TileState::Idle;
            }
        }
    }

    /**
     * @brief Drives frame logic cycles; updates audio streams, interprets user interactions, steps active animations, and clears expired layouts.
     */
    void Update() {
        UpdateMusicStream(background_music_);
        HandleInput(GetMousePosition());
        ProcessFallAnimations();
        ProcessMatchDelay();

        for (auto &popup : score_popups_) {
            popup.lifetime -= GetFrameTime();
            popup.position.y -= 30.0F * GetFrameTime(); // Floating displacement vector transformation step scale velocity
            popup.alpha = popup.lifetime;
        }

        std::erase_if(score_popups_, [](const ScorePopup &popup) {
            return popup.lifetime <= 0.0F;
        });

        if (score_animating_) {
            score_scale_ += score_scale_velocity_ * GetFrameTime();
            if (score_scale_ <= 1.0F) {
                score_scale_ = 1.0F;
                score_animating_ = false;
            }
        }
    }

    /**
     * @brief Evaluates assets, board configurations, and active user interface layers to assemble the final output viewport.
     */
    void Draw() {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(background_, Rectangle{.x = 0.0F, .y = 0.0F, .width = static_cast<float>(background_.width), .height = static_cast<float>(background_.height)}, Rectangle{.x = 0.0F, .y = 0.0F, .width = static_cast<float>(GetScreenWidth()), .height = static_cast<float>(GetScreenHeight())}, Vector2{.x = 0.0F, .y = 0.0F}, 0.0F, WHITE);
        DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(DARKBLUE, 0.40F), Fade(BLACK, 0.80F));

        DrawRectangleRounded(Rectangle{.x = grid_origin_.x, .y = grid_origin_.y, .width = 336.0F, .height = 336.0F}, 0.2F, 8, Fade(DARKGRAY, 0.60F));
        // 0.2F roundness multiplier; 8 corner tessellation detail loops; 60% background transparency layer blend
        // 8 column * 42.0F grid footprint allocation mapping spacing dimensions

        for (auto row = static_cast<std::size_t>(0); row < 8; row++) {
            for (auto col = static_cast<std::size_t>(0); col < 8; col++) {
                auto rect = Rectangle{.x = grid_origin_.x + (static_cast<float>(col) * 42.0F), .y = grid_origin_.y + (static_cast<float>(row) * 42.0F), .width = 42.0F, .height = 42.0F};

                DrawRectangleRoundedLinesEx(rect, 0.2F, 8, 1.0F, DARKGRAY); // 1.0F stroke thickness border rendering trace layout

                if (board_.at(row).at(col) != ' ') {
                    const auto *text = TextFormat("%c", board_.at(row).at(col));
                    auto text_size = MeasureTextEx(GetFontDefault(), text, 20.0F, 1.0F); // Default glyph size tracking context boundary mappings at 20.0pt line sizing scale heights

                    // Sub-quadrant grid center step normalization transformations
                    auto text_pos = Vector2{.x = rect.x + ((42.0F - text_size.x) / 2.0F), .y = rect.y + ((42.0F - text_size.y) / 2.0F) - fall_offset_.at(row).at(col)};

                    DrawTextEx(GetFontDefault(), text, text_pos, 20.0F, 1.0F, matched_.at(row).at(col) ? GREEN : WHITE);
                }
            }
        }

        if (selected_tile_.x >= 0.0F) {
            // 2.0F highlighting outline marker line dimension width setting metrics tracking parameters active choices
            DrawRectangleRoundedLinesEx(Rectangle{.x = grid_origin_.x + (selected_tile_.x * 42.0F), .y = grid_origin_.y + (selected_tile_.y * 42.0F), .width = 42.0F, .height = 42.0F}, 0.2F, 8, 2.0F, YELLOW);
        }

        // Screen layout margin positioning coordinates vector trace points
        DrawTextEx(score_font_, TextFormat("SCORE: %d", score_), Vector2{.x = 20.0F, .y = 20.0F}, 32.0F * score_scale_, 1.0F, YELLOW);

        for (const auto &popup : score_popups_) {
            auto alpha_color = Fade(YELLOW, popup.alpha);
            DrawText(TextFormat("+%d", popup.amount), static_cast<int>(popup.position.x), static_cast<int>(popup.position.y), 20, alpha_color); // 20pt display text spacing format constraints matching floating dynamic objects bounds mappings maps
        }

        EndDrawing();
    }
};

} // namespace
} // namespace MatchGame

int main() {
    MatchGame::Game game(800, 450, "Raylib 2D ASCII MATCH (Modern C++23)");
    game.Run();
    return 0;
}
