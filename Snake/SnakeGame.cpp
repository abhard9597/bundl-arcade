#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/sound.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>
#include <wx/filefn.h>

#include <deque>
#include <random>
#include <algorithm>
#include <cmath>
#include <map>
#include <fstream>
#include <sstream>
#include <limits>

#include "Theme.h"
#include "PauseMenu.h"
#include "MainMenu.h"
#include "Model.h"
#include "Enemy.h"

static bool LauncherDarkModeEnabled()
{
    wxString value;
    if (!wxGetEnv("BUNDL_DARK_MODE", &value))
        return true; // default dark

    return value == "1" || value.CmpNoCase("true") == 0;
}

static void PlayWavPortable(const wxString &fileName)
{
#ifdef __linux__
    // Resolve WAV path next to the executable
    static bool s_inited = false;
    static wxString s_basePath;

    if (!s_inited)
    {
        wxFileName exe(wxStandardPaths::Get().GetExecutablePath());
        s_basePath = exe.GetPath();
        s_inited = true;
    }

    wxString full = wxFileName(s_basePath, fileName).GetFullPath();

    wxString cmd = "paplay \"" + full + "\"";
    wxExecute(cmd, wxEXEC_ASYNC);
#else
    static std::map<wxString, wxSound> s_cache;

    auto it = s_cache.find(fileName);
    if (it == s_cache.end())
    {
        wxFileName exe(wxStandardPaths::Get().GetExecutablePath());
        wxString full = wxFileName(exe.GetPath(), fileName).GetFullPath();

        // Construct wxSound directly in the map
        auto tryInsert = [&](const wxString &path) -> std::map<wxString, wxSound>::iterator
        {
            auto [iter, inserted] = s_cache.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(fileName),
                std::forward_as_tuple(path));
            if (!iter->second.IsOk())
            {
                s_cache.erase(iter);
                return s_cache.end();
            }
            return iter;
        };

        it = tryInsert(full);
        if (it == s_cache.end())
        {
            it = tryInsert(fileName);
            if (it == s_cache.end())
                return;
        }
    }

    wxSound &snd = it->second;
    snd.Play(wxSOUND_ASYNC);
#endif
}

// Model types live in Model.h

struct Food
{
    Point pos;
};

struct Score
{
    int value = 0;
    void Reset() { value = 0; }
};

// Snake lives in Model.h

struct GameConfig
{
    int tickMs = 120;
    int growthPerFood = 1;
    bool wrapWalls = false;
    int foodTtlTicks = 0;
};

// Snake type selection is based on keywords in the player name.
// (You can later replace this with an explicit selection UI if you want.)
static SnakeType DetermineTypeFromName(const wxString &nameRaw)
{
    wxString name = nameRaw.Lower().Trim(true).Trim(false);
    if (name.Contains("speed") || name.Contains("fast"))
        return SnakeType::Speedy;
    if (name.Contains("tank") || name.Contains("heavy"))
        return SnakeType::Tank;
    if (name.Contains("ghost") || name.Contains("shade"))
        return SnakeType::Ghost;

    // Stable fallback: hash the name into one of the types.
    unsigned hash = 2166136261u;
    for (unsigned char ch : name.ToStdString())
    {
        hash ^= ch;
        hash *= 16777619u;
    }
    switch (hash % 4u)
    {
    case 0:
        return SnakeType::Classic;
    case 1:
        return SnakeType::Speedy;
    case 2:
        return SnakeType::Tank;
    default:
        return SnakeType::Ghost;
    }
}

enum class Difficulty
{
    Easy,
    Normal,
    Hard,
    Insane
};

static double DifficultySpeedFactor(Difficulty d)
{
    switch (d)
    {
    case Difficulty::Easy:
        return 1.15;
    case Difficulty::Normal:
        return 1.00;
    case Difficulty::Hard:
        return 0.85;
    case Difficulty::Insane:
        return 0.70;
    }
    return 1.0;
}

static int DifficultyFoodTtlTicks(Difficulty d)
{
    switch (d)
    {
    case Difficulty::Easy:
        return 0;
    case Difficulty::Normal:
        return 0;
    case Difficulty::Hard:
        return 45;
    case Difficulty::Insane:
        return 25;
    }
    return 0;
}

static wxString DifficultyLabel(Difficulty d)
{
    switch (d)
    {
    case Difficulty::Easy:
        return "Easy";
    case Difficulty::Normal:
        return "Normal";
    case Difficulty::Hard:
        return "Hard";
    case Difficulty::Insane:
        return "Insane";
    }
    return "Normal";
}

static wxString SkinLabel(SkinId id)
{
    switch (id)
    {
    case SkinId::Classic:
        return "Classic";
    case SkinId::Light:
        return "Light";
    case SkinId::Neon:
        return "Neon";
    case SkinId::Amber:
        return "Amber";
    case SkinId::Mono:
        return "Monochrome";
    case SkinId::CRT:
        return "CRT";
    case SkinId::AdvancedGrid:
        return "Persona 5 Grid";
    }
    return "Classic";
}

struct GameState
{
    Board board;
    Snake snake;
    Snake enemy;
    bool enemyEnabled = false;
    Food food;
    Score score;

    bool gameOver = false;
    std::mt19937 rng{std::random_device{}()};
    GameConfig cfg;

    // Type + Tank forgiveness state
    SnakeType type = SnakeType::Classic;
    int tankArmorHitsLeft = 0;        // wall hits that can be absorbed (Tank)
    int tankSelfBiteForgivesLeft = 0; // self-collisions that can be forgiven (Tank)
    int tankRecoverTicks = 0;         // Tank: pause next tick to let player turn after a forgiveness

    int foodAgeTicks = 0;

    void Reset(const wxString &playerName, SnakeType st, Difficulty diff, bool wrapEnabled)
    {
        score.Reset();
        gameOver = false;

        // Insane mode: double the playfield and enable a powerful enemy.
        if (diff == Difficulty::Insane)
        {
            board.w = 30;
            board.h = 20;
            enemyEnabled = true;
        }
        else
        {
            board.w = 20;
            board.h = 10;
            enemyEnabled = false;
        }

        type = st;
        SnakeTypeInfo info = GetSnakeTypeInfo(type);

        cfg.tickMs = std::max(25, (int)std::lround(info.baseTickMs * DifficultySpeedFactor(diff)));
        cfg.growthPerFood = info.growthPerFood;
        cfg.foodTtlTicks = DifficultyFoodTtlTicks(diff);

        // Wrap behavior:
        // - If the menu option is ON, everyone wraps.
        // - If the type has defaultWrap (Ghost), it wraps even when the option is OFF.
        cfg.wrapWalls = wrapEnabled || info.defaultWrap;

        // Tank forgiveness counters.
        tankArmorHitsLeft = info.armorWallHits;
        tankSelfBiteForgivesLeft = info.selfBiteForgives;
        tankRecoverTicks = 0;

        snake.body.clear();
        snake.dir = Dir::Right;
        snake.growPending = false;

        int cx = board.w / 2, cy = board.h / 2;
        for (int i = 0; i < 4; ++i)
            snake.body.push_back({cx - i, cy});

        enemy.body.clear();
        enemy.dir = Dir::Left;
        enemy.growPending = false;
        if (enemyEnabled)
        {
            // Start enemy away from the player.
            int ex = board.w / 2;
            int ey = board.h / 2;
            ex += board.w / 4;
            for (int i = 0; i < 4; ++i)
                enemy.body.push_back({ex + i, ey});
        }

        SpawnFood();
    }

    void SpawnFood()
    {
        std::uniform_int_distribution<int> dx(0, board.w - 1);
        std::uniform_int_distribution<int> dy(0, board.h - 1);

        for (int tries = 0; tries < 5000; ++tries)
        {
            Point p{dx(rng), dy(rng)};
            if (!snake.Occupies(p) && (!enemyEnabled || !enemy.Occupies(p)))
            {
                food.pos = p;
                foodAgeTicks = 0;
                return;
            }
        }
        gameOver = true;
    }

    Point WrapIfNeeded(Point p) const
    {
        if (!cfg.wrapWalls)
            return p;
        if (p.x < 0)
            p.x = board.w - 1;
        if (p.x >= board.w)
            p.x = 0;
        if (p.y < 0)
            p.y = board.h - 1;
        if (p.y >= board.h)
            p.y = 0;
        return p;
    }

    bool WouldHitSelf(const Point &nh) const
    {
        if (snake.body.empty())
            return false;

        if (!snake.growPending)
        {
            for (size_t i = 0; i + 1 < snake.body.size(); ++i)
                if (snake.body[i] == nh)
                    return true;
            return false;
        }
        return snake.Occupies(nh);
    }

    bool WouldHitEnemy(const Point &nh) const
    {
        if (!enemyEnabled)
            return false;
        // Allow stepping into enemy tail if it is going to move.
        if (!enemy.growPending && !enemy.body.empty())
        {
            for (size_t i = 0; i + 1 < enemy.body.size(); ++i)
                if (enemy.body[i] == nh)
                    return true;
            return false;
        }
        return enemy.Occupies(nh);
    }

    bool EnemyWouldHitSelf(const Point &nh) const
    {
        if (enemy.body.empty())
            return false;
        if (!enemy.growPending)
        {
            for (size_t i = 0; i + 1 < enemy.body.size(); ++i)
                if (enemy.body[i] == nh)
                    return true;
            return false;
        }
        return enemy.Occupies(nh);
    }

    bool EnemyWouldHitPlayer(const Point &nh) const
    {
        // Allow stepping into player's tail if it is going to move.
        if (!snake.growPending && !snake.body.empty())
        {
            for (size_t i = 0; i + 1 < snake.body.size(); ++i)
                if (snake.body[i] == nh)
                    return true;
            return false;
        }
        return snake.Occupies(nh);
    }

    struct TickResult
    {
        bool playerAte = false;
        bool enemyAte = false;
        bool scoreMultipleOf10 = false;

        // Optional HUD pop events (Tank mechanics)
        bool tankWallAbsorbed = false; // consumed armor wall hit
        bool tankSelfForgiven = false; // consumed self-bite forgiveness
        int scorePenalty = 0;          // e.g. 3 for wall hit
        Point eventAt{-999, -999};     // where to show the pop text

        Point ateAt{-999, -999};
        int newScore = 0;
    };

    TickResult Tick()
    {
        TickResult res;
        if (gameOver)
            return res;

        if (cfg.foodTtlTicks > 0)
        {
            ++foodAgeTicks;
            if (foodAgeTicks >= cfg.foodTtlTicks)
                SpawnFood();
        }

        // Tank forgiveness: after an absorbed hit / warning, pause one tick so the player can turn.
        if (type == SnakeType::Tank && tankRecoverTicks > 0)
        {
            --tankRecoverTicks;
            return res;
        }

        // Decide enemy direction before stepping. Insane only.
        if (enemyEnabled && !enemy.body.empty())
        {
            enemy.dir = EnemyAI::ChooseDir(board, enemy, snake, food.pos, cfg.wrapWalls);
        }

        Point pNext = WrapIfNeeded(snake.NextHead());
        Point eNext = enemyEnabled ? WrapIfNeeded(enemy.NextHead()) : Point{-999, -999};

        // Wall collision
        if (!cfg.wrapWalls && !board.IsInside(pNext))
        {
            // Tank can absorb one wall hit (costs points) instead of dying.
            if (type == SnakeType::Tank && tankArmorHitsLeft > 0)
            {
                SnakeTypeInfo info = GetSnakeTypeInfo(type);
                // If score can't pay the penalty, you die.
                if (score.value < info.wallScorePenalty)
                {
                    gameOver = true;
                    return res;
                }

                score.value -= info.wallScorePenalty;
                tankArmorHitsLeft--;

                res.tankWallAbsorbed = true;
                res.scorePenalty = info.wallScorePenalty;
                // Show pop near current head (still inside).
                res.eventAt = snake.Head();
                tankRecoverTicks = 1;

                // Don't move this tick.
                return res;
            }

            gameOver = true;
            return res;
        }

        if (enemyEnabled && !cfg.wrapWalls && !board.IsInside(eNext))
        {
            gameOver = true;
            return res;
        }

        // Head-on collision
        if (enemyEnabled && pNext == eNext)
        {
            gameOver = true;
            return res;
        }

        // Body collisions
        if (WouldHitSelf(pNext))
        {
            // Tank can forgive one self-collision (warning). Second kills.
            if (type == SnakeType::Tank && tankSelfBiteForgivesLeft > 0)
            {
                tankSelfBiteForgivesLeft--;
                res.tankSelfForgiven = true;
                res.eventAt = snake.Head();
                tankRecoverTicks = 1;
                // Don't move this tick.
                return res;
            }
            gameOver = true;
            return res;
        }
        if (WouldHitEnemy(pNext))
        {
            gameOver = true;
            return res;
        }

        if (enemyEnabled && (EnemyWouldHitSelf(eNext) || EnemyWouldHitPlayer(eNext)))
        {
            gameOver = true;
            return res;
        }

        // Eating resolution. If both go for the food, enemy wins on Insane
        bool pAte = (pNext == food.pos);
        bool eAte = enemyEnabled && (eNext == food.pos);
        if (pAte && eAte)
            pAte = false;

        if (pAte)
        {
            score.value += 1;
            for (int i = 0; i < cfg.growthPerFood; ++i)
                snake.Grow();

            res.playerAte = true;
            res.ateAt = pNext;
            res.newScore = score.value;
            res.scoreMultipleOf10 = (score.value > 0 && (score.value % 10 == 0));
        }
        if (eAte)
        {
            for (int i = 0; i < std::max(1, cfg.growthPerFood); ++i)
                enemy.Grow();
            res.enemyAte = true;
        }

        // Advance
        snake.AdvanceTo(pNext);
        if (enemyEnabled)
            enemy.AdvanceTo(eNext);

        if (pAte || eAte)
            SpawnFood();

        return res;
    }
};

#include "SnakeGame.h"
#include "TypeSelect.h"

// UI: GamePanel

class GamePanel final : public wxPanel
{
public:
    explicit GamePanel(wxWindow *parent)
        : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS),
          timer_(this)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);

        Bind(wxEVT_PAINT, &GamePanel::OnPaint, this);
        Bind(wxEVT_KEY_DOWN, &GamePanel::OnKeyDown, this);
        Bind(wxEVT_TIMER, &GamePanel::OnTimer, this);

        Bind(wxEVT_MOTION, &GamePanel::OnMouseMove, this);
        Bind(wxEVT_LEFT_DOWN, &GamePanel::OnMouseDown, this);
        Bind(wxEVT_LEFT_UP, &GamePanel::OnMouseUp, this);

        CallAfter([this]
                  {
            this->SetFocus();
            this->SetFocusFromKbd(); });

        playerName_ = "Player";
        skinId_ = LauncherDarkModeEnabled() ? SkinId::Classic : SkinId::Light;
        diff_ = Difficulty::Normal;
        wrapEnabled_ = false;
        paused_ = false;

        // Load persisted high score for the initial name.
        SyncHighScoreFromDisk_();

        // Load global leaderboard (best score across all players).
        LoadGlobalHighScore_(&globalHighScore_, &globalHighName_);

        // Pause Menu actions
        pauseMenu_.SetActions(
            [this]()
            { this->SetPaused(false); },
            [this]()
            { this->NewGame(); },
            [this]()
            { this->ShowStatsDialog_(); },
            [this]()
            { this->ShowControlsDialog_(); },
            [this]()
            { this->ShowTutorialDialog_(); },
            [this]()
            { this->GoToMainMenu_(); });

        // Pause options: Wrap, Difficulty, Skin, Sound
        pauseMenu_.SetOptions(
            // Toggle wrap
            [this]()
            {
                wrapEnabled_ = !wrapEnabled_;
                ApplyAndRestartKeepPaused_();
            },
            // Cycle difficulty
            [this]()
            {
                switch (diff_)
                {
                case Difficulty::Easy:
                    diff_ = Difficulty::Normal;
                    break;
                case Difficulty::Normal:
                    diff_ = Difficulty::Hard;
                    break;
                case Difficulty::Hard:
                    diff_ = Difficulty::Insane;
                    break;
                case Difficulty::Insane:
                    diff_ = Difficulty::Easy;
                    break;
                }
                ApplyAndRestartKeepPaused_();
            },
            // Cycle skin
            [this]()
            {
                CycleSkin_();
            },
            // Toggle sound
            [this]()
            {
                ToggleSound_();
            });

        pauseMenu_.SetSfxCallbacks(
            [this]()
            { PlaySfx_("00003_streaming.wav"); }, // hover over setting
            [this]()
            { PlaySfx_("00007.wav"); } // click setting button
        );

        // Main Menu actions (Start / Stats / Tutorials / Exit)
        mainMenu_.SetActions(
            // Start Game
            [this]()
            {
                if (!PromptForPlayerNameBeforeStart_())
                    return;
                BeginBiteToGame_(true);
            },
            // Stats
            [this]()
            {
                this->ShowGlobalStatsDialog_();
            },
            // Tutorials
            [this]()
            {
                ShowTutorialDialog_();
            },
            // Exit
            [this]()
            {
                if (auto *f = wxDynamicCast(wxGetTopLevelParent(this), wxFrame))
                    f->Close(true);
            });

        // Main menu options: Theme + Difficulty + Sound
        mainMenu_.SetOptions(
            [this]()
            { CycleSkin_(); },
            [this]()
            {
                switch (diff_)
                {
                case Difficulty::Easy:
                    diff_ = Difficulty::Normal;
                    break;
                case Difficulty::Normal:
                    diff_ = Difficulty::Hard;
                    break;
                case Difficulty::Hard:
                    diff_ = Difficulty::Insane;
                    break;
                case Difficulty::Insane:
                    diff_ = Difficulty::Easy;
                    break;
                }
                Refresh(false);
                SetFocus();
            },
            [this]()
            { ToggleSound_(); });

        // Same SFX as pause menu
        mainMenu_.SetSfxCallbacks(
            [this]()
            { PlaySfx_("00003_streaming.wav"); }, // hover
            [this]()
            { PlaySfx_("00007.wav"); } // click
        );

        // Initialize state without starting countdown/gameplay yet
        state_.Reset(playerName_, playerType_, diff_, wrapEnabled_);
        countdownActive_ = false;
        countdownMsLeft_ = 0;
        accumMs_ = 0;

        // Start on main menu
        uiMode_ = UiMode::MainMenu;
        mainMenu_.Open();

        // Run at ~60fps so menus animate smoothly
        timer_.Start(16);
        SetFocus();
    }

    void NewGame()
    {
        SetPaused(false);
        ApplyAndRestart();
    }

    void TogglePause()
    {
        // Don't allow pause toggling if we're in main menu or transitioning
        if (uiMode_ != UiMode::Playing)
            return;
        SetPaused(!paused_);
    }

private:
    // Speed-up tuning:
    // - smaller tickMs == faster
    static constexpr int kMinTickMs = 35;    // fastest allowed
    static constexpr int kSpeedupStepMs = 2; // speed gained per food

    enum class UiMode
    {
        MainMenu,
        Playing,
        TransitionToGame
    };

    GameState state_;
    wxTimer timer_;
    wxString playerName_;
    SnakeType playerType_ = SnakeType::Classic;
    SkinId skinId_;
    Difficulty diff_;
    bool wrapEnabled_;
    bool paused_;
    int playerHighScore_ = 0;

    // Best score across all players (displayed on main menu)
    int globalHighScore_ = 0;
    wxString globalHighName_ = "—";

    // global sound toggle
    bool soundEnabled_ = true;

    PauseMenu pauseMenu_;
    MainMenu mainMenu_;

    UiMode uiMode_ = UiMode::MainMenu;

    // Bite transition
    bool biteActive_ = false;
    float biteT_ = 0.f;        // 0..1
    int biteDurationMs_ = 900; // ~0.9s
    bool biteStartNew_ = true; // true = ApplyAndRestart(), false = keep loaded state

    // Start-of-run countdown so the game doesn't kick off instantly.
    bool countdownActive_ = false;
    int countdownMsLeft_ = 0; // milliseconds

    // Smooth stepping, run render loop at 16ms, advance game at cfg.tickMs
    int accumMs_ = 0;

    // Effects
    int frameCounter_ = 0;
    int scorePulseTicks_ = 0;

    // Indicator, floating text near the eaten cell.
    float popLife_ = 0.f;
    wxPoint popPos_{0, 0};
    wxString popText_;

    // Game-over clickable buttons (computed in OnPaint)
    wxRect gameOverRestartBtn_{0, 0, 0, 0};
    wxRect gameOverMenuBtn_{0, 0, 0, 0};

    // helpers

    void PlaySfx_(const wxString &wav)
    {
        if (!soundEnabled_)
            return;
        PlayWavPortable(wav);
    }

    // --- High score persistence (auto-save best score) ---
    static wxString SanitizeFileToken_(const wxString &in)
    {
        wxString out;
        out.reserve(in.size());
        for (wxUniChar c : in)
        {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
                out.Append(c);
            else
                out.Append('_');
        }
        if (out.IsEmpty())
            out = "Player";
        return out;
    }

    static wxString HighScorePathFor_(const wxString &playerName)
    {
        wxString base = wxStandardPaths::Get().GetUserLocalDataDir();
        wxFileName::Mkdir(base, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
        wxString token = SanitizeFileToken_(playerName);
        return wxFileName(base, wxString::Format("snake_highscore_%s.txt", token)).GetFullPath();
    }

    static int LoadHighScoreFor_(const wxString &playerName)
    {
        std::ifstream f(HighScorePathFor_(playerName).ToStdString());
        if (!f.is_open())
            return 0;
        int v = 0;
        f >> v;
        return std::max(0, v);
    }

    static void SaveHighScoreFor_(const wxString &playerName, int score)
    {
        std::ofstream f(HighScorePathFor_(playerName).ToStdString(), std::ios::out | std::ios::trunc);
        if (!f.is_open())
            return;
        f << score;
    }

    // --- Global leaderboard persistence (best score across all players) ---
    static wxString GlobalHighScorePath_()
    {
        wxString base = wxStandardPaths::Get().GetUserLocalDataDir();
        wxFileName::Mkdir(base, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
        return wxFileName(base, "snake_global_highscore.txt").GetFullPath();
    }

    static void LoadGlobalHighScore_(int *outScore, wxString *outName)
    {
        if (outScore)
            *outScore = 0;
        if (outName)
            *outName = "—";

        std::ifstream f(GlobalHighScorePath_().ToStdString());
        if (!f.is_open())
            return;

        int s = 0;
        std::string nameLine;
        f >> s;
        f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::getline(f, nameLine);

        if (outScore)
            *outScore = std::max(0, s);
        if (outName)
            *outName = nameLine.empty() ? wxString("—") : wxString(nameLine);
    }

    static void SaveGlobalHighScore_(int score, const wxString &name)
    {
        std::ofstream f(GlobalHighScorePath_().ToStdString(), std::ios::out | std::ios::trunc);
        if (!f.is_open())
            return;
        f << score << "\n"
          << name.ToStdString() << "\n";
    }
    void SyncHighScoreFromDisk_()
    {
        playerHighScore_ = LoadHighScoreFor_(playerName_);
    }

    void MaybeCommitHighScore_(int newScore)
    {
        if (newScore > playerHighScore_)
        {
            playerHighScore_ = newScore;
            SaveHighScoreFor_(playerName_, playerHighScore_);
        }

        // Update global leaderboard.
        if (newScore > globalHighScore_)
        {
            globalHighScore_ = newScore;
            globalHighName_ = playerName_;
            SaveGlobalHighScore_(globalHighScore_, globalHighName_);
        }
    }

    void ToggleSound_()
    {
        soundEnabled_ = !soundEnabled_;
        Refresh(false);
        SetFocus();
    }

    void GoToMainMenu_()
    {
        // Leave gameplay and return to the main menu (do NOT close the app).
        paused_ = false;
        pauseMenu_.Close();
        biteActive_ = false;
        biteT_ = 0.f;
        countdownActive_ = false;
        countdownMsLeft_ = 0;
        accumMs_ = 0;
        uiMode_ = UiMode::MainMenu;
        mainMenu_.Open();
        Refresh(false);
        SetFocus();
    }

    void CycleSkin_()
    {
        switch (skinId_)
        {
        case SkinId::Classic:
            skinId_ = SkinId::Light;
            break;
        case SkinId::Light:
            skinId_ = SkinId::Neon;
            break;
        case SkinId::Neon:
            skinId_ = SkinId::Amber;
            break;
        case SkinId::Amber:
            skinId_ = SkinId::Mono;
            break;
        case SkinId::Mono:
            skinId_ = SkinId::CRT;
            break;
        case SkinId::CRT:
            skinId_ = SkinId::AdvancedGrid;
            break;
        case SkinId::AdvancedGrid:
            skinId_ = SkinId::Classic;
            break;
        }
        Refresh(false);
        SetFocus();
    }

    void ApplyAndRestart()
    {
        state_.Reset(playerName_, playerType_, diff_, wrapEnabled_);
        accumMs_ = 0;
        scorePulseTicks_ = 0;
        popLife_ = 0.f;

        countdownActive_ = true;
        countdownMsLeft_ = 3000;
        Refresh(false);
        SetFocus();
    }

    void ApplyAndRestartKeepPaused_()
    {
        state_.Reset(playerName_, playerType_, diff_, wrapEnabled_);
        accumMs_ = 0;
        scorePulseTicks_ = 0;
        popLife_ = 0.f;

        countdownActive_ = true;
        countdownMsLeft_ = 3000;
        Refresh(false);
        SetFocus();
    }

    void SetPaused(bool p)
    {
        paused_ = p;
        if (paused_)
            pauseMenu_.Open();
        else
            pauseMenu_.Close();
        Refresh(false);
        SetFocus();
    }

    // Main Menu -> Game Bite Transition

    void BeginBiteToGame_(bool startNew)
    {
        // Lock out pause while transitioning
        paused_ = false;
        pauseMenu_.Close();

        biteActive_ = true;
        biteT_ = 0.f;
        biteStartNew_ = startNew;

        uiMode_ = UiMode::TransitionToGame;
        mainMenu_.Close();
        Refresh(false);
        SetFocus();
    }

    void DrawBiteOverlay_(wxGraphicsContext *gc, const wxRect &screen, float t)
    {
        // "swallow" bite circle expanding with jagged teeth
        t = std::clamp(t, 0.f, 1.f);

        constexpr double kPi = 3.14159265358979323846;
        const double diag = std::hypot((double)screen.width, (double)screen.height);
        const double R = (0.18 + 0.95 * t) * diag;

        const double cx = screen.x + screen.width * 0.50;
        const double cy = screen.y + screen.height * 0.52;

        const int teeth = 44;
        const double toothAmp = 18.0 + 12.0 * std::sin(frameCounter_ * 0.08);

        wxGraphicsPath p = gc->CreatePath();

        for (int i = 0; i <= teeth; ++i)
        {
            double a = (i / (double)teeth) * (2.0 * kPi);
            double alt = (i % 2 == 0) ? 1.0 : -1.0;
            double rr = R + alt * toothAmp;

            double x = cx + std::cos(a) * rr;
            double y = cy + std::sin(a) * rr;

            if (i == 0)
                p.MoveToPoint(x, y);
            else
                p.AddLineToPoint(x, y);
        }
        p.CloseSubpath();

        // Fill bite shape
        gc->SetBrush(wxBrush(wxColour(0, 0, 0, 235)));
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->FillPath(p);

        // Rim
        if (t < 0.96f)
        {
            wxColour rim(255, 0, 60, (int)(120 * (1.0 - t)));
            gc->SetPen(wxPen(rim, 4));
            gc->StrokePath(p);
        }

        // Overall darken
        int fade = (int)std::lround(140 * t);
        gc->SetBrush(wxBrush(wxColour(0, 0, 0, fade)));
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->DrawRectangle(screen.x, screen.y, screen.width, screen.height);
    }

    // UI / Input

    bool PromptForPlayerNameBeforeStart_()
    {
        wxTextEntryDialog dlg(this,
                              "Type your name.",
                              "Player Name",
                              playerName_);

        if (dlg.ShowModal() != wxID_OK)
        {
            SetFocus();
            return false;
        }

        playerName_ = dlg.GetValue().IsEmpty() ? "Player" : dlg.GetValue();

        // Pick a type after the name is entered.
        {
            Skin skin = GetSkin(skinId_);
            SnakeType chosen;
            if (!RunTypeSelectDialog(this, skin, playerType_, &chosen))
            {
                SetFocus();
                return false;
            }
            playerType_ = chosen;
        }

        // Auto-load stored best score for this name.
        SyncHighScoreFromDisk_();

        // Refresh baseline state shown behind menus.
        state_.Reset(playerName_, playerType_, diff_, wrapEnabled_);
        Refresh(false);
        SetFocus();
        return true;
    }

    void ShowGlobalStatsDialog_()
    {
        // Refresh from disk in case the file was updated outside this session.
        LoadGlobalHighScore_(&globalHighScore_, &globalHighName_);

        wxString msg;
        if (globalHighScore_ <= 0)
        {
            msg = "No global high score yet.\n\nPlay a run to set the first record!";
        }
        else
        {
            msg << "Highest Score: " << globalHighScore_ << "\n"
                << "Player: " << globalHighName_ << "\n";
        }

        wxMessageDialog dlg(this, msg, "Global High Score", wxOK | wxICON_INFORMATION);
        dlg.ShowModal();
        SetFocus();
    }

    void ShowStatsDialog_()
    {
        SnakeTypeInfo info = GetSnakeTypeInfo(playerType_);
        wxString typeStr = wxString(SnakeTypeName(playerType_));
        wxString msg;
        msg << "Player: " << playerName_ << "\n"
            << "Type: " << typeStr << "\n"
            << "Type Perk: " << wxString(info.desc) << "\n"
            << "Skin: " << SkinLabel(skinId_) << "\n"
            << "Difficulty: " << DifficultyLabel(diff_) << "\n"
            << "Wrap Walls: " << (wrapEnabled_ ? "ON" : "OFF") << "\n"
            << "Sound: " << (soundEnabled_ ? "ON" : "OFF") << "\n"
            << "High Score: " << playerHighScore_ << "\n";

        if (state_.cfg.foodTtlTicks > 0)
            msg << "Food expiry: " << state_.cfg.foodTtlTicks << " ticks\n";
        else
            msg << "Food expiry: None\n";

        wxMessageDialog dlg(this, msg, "Stats", wxOK | wxICON_INFORMATION);
        dlg.ShowModal();
        SetFocus();
    }

    void ShowControlsDialog_()
    {
        wxString msg;
        msg << "Movement:\n"
            << "  Arrow Keys  - Move\n\n"
            << "Meta:\n"
            << "  P           - Pause/Resume\n"
            << "  R           - Restart\n"
            << "  Enter       - Restart on Game Over\n";

        wxMessageDialog dlg(this, msg, "Controls", wxOK | wxICON_INFORMATION);
        dlg.ShowModal();
        SetFocus();
    }

    void ShowTutorialDialog_()
    {
        wxString msg;
        msg << "Goal:\n"
            << "  Eat food to grow and increase your score.\n\n"
            << "Avoid:\n"
            << "  Hitting walls (unless Wrap Walls is ON).\n"
            << "  Hitting your own body.\n\n"
            << "Difficulty:\n"
            << "  Hard/Insane may spawn food that expires.\n"
            << "  Insane adds an enemy snake.\n";

        wxMessageDialog dlg(this, msg, "Tutorial", wxOK | wxICON_INFORMATION);
        dlg.ShowModal();
        SetFocus();
    }

    void OnTimer(wxTimerEvent &)
    {
        ++frameCounter_;

        // Always tick menu animations
        pauseMenu_.Tick();
        mainMenu_.Tick();

        // Transition: bite swallow
        if (uiMode_ == UiMode::TransitionToGame && biteActive_)
        {
            biteT_ += 16.0f / (float)biteDurationMs_;
            if (biteT_ >= 1.f)
            {
                biteT_ = 1.f;
                biteActive_ = false;

                uiMode_ = UiMode::Playing;

                if (biteStartNew_)
                {
                    ApplyAndRestart();
                }
                else
                {
                    if (!countdownActive_)
                    {
                        countdownActive_ = true;
                        countdownMsLeft_ = 3000;
                    }
                }
                SetFocus();
            }

            Refresh(false);
            return; // don't tick gameplay during transition
        }

        // Main menu: no gameplay ticking
        if (uiMode_ == UiMode::MainMenu)
        {
            accumMs_ = 0;
            Refresh(false);
            return;
        }

        // Playing:
        if (!paused_ && countdownActive_)
        {
            countdownMsLeft_ -= 16;
            if (countdownMsLeft_ <= 0)
            {
                countdownMsLeft_ = 0;
                countdownActive_ = false;
            }
        }

        if (!paused_ && !state_.gameOver && !countdownActive_)
        {
            accumMs_ += 16;
            while (accumMs_ >= state_.cfg.tickMs)
            {
                const int tickUsed = state_.cfg.tickMs;

                int before = state_.score.value;
                auto tr = state_.Tick();
                int after = state_.score.value;

                // Tank forgiveness events: stop ticking further this frame so we don't immediately
                // consume the forgiveness and then die in the same render frame.
                if (tr.tankWallAbsorbed)
                {
                    popLife_ = 1.f;
                    popText_ = wxString::Format("-%d", tr.scorePenalty);
                    popPos_ = wxPoint(tr.eventAt.x, tr.eventAt.y);
                    accumMs_ = 0;
                    break;
                }
                if (tr.tankSelfForgiven)
                {
                    popLife_ = 1.f;
                    popText_ = "WARNING";
                    popPos_ = wxPoint(tr.eventAt.x, tr.eventAt.y);
                    accumMs_ = 0;
                    break;
                }

                if (after > before)
                {
                    MaybeCommitHighScore_(after);
                    scorePulseTicks_ = 12;

                    popLife_ = 1.f;
                    popText_ = tr.scoreMultipleOf10 ? wxString::Format("%d!", tr.newScore) : "+1";
                    popPos_ = wxPoint(tr.ateAt.x, tr.ateAt.y);

                    // Gated by soundEnabled_
                    if (tr.scoreMultipleOf10)
                        PlaySfx_("00045.wav");
                    else
                        PlaySfx_("00046.wav");
                }

                // Speed up when either snake eats
                if (tr.playerAte || tr.enemyAte)
                {
                    state_.cfg.tickMs = std::max(kMinTickMs, state_.cfg.tickMs - kSpeedupStepMs);
                }

                accumMs_ -= tickUsed;
            }
        }
        else
        {
            accumMs_ = 0;
        }

        if (scorePulseTicks_ > 0)
            --scorePulseTicks_;
        if (popLife_ > 0.f)
            popLife_ = std::max(0.f, popLife_ - 0.06f);

        Refresh(false);
    }

    void OnKeyDown(wxKeyEvent &e)
    {
        int k = e.GetKeyCode();

        // Main Menu navigation
        if (uiMode_ == UiMode::MainMenu)
        {
            mainMenu_.OnKeyDown(k);
            Refresh(false);
            return;
        }

        // Transition: ignore input
        if (uiMode_ == UiMode::TransitionToGame)
            return;

        // Playing:
        if (k == 'P' || k == 'p')
        {
            TogglePause();
            return;
        }
        if (k == 'R' || k == 'r')
        {
            NewGame();
            return;
        }

        if (paused_)
        {
            pauseMenu_.OnKeyDown(k);
            Refresh(false);
            return;
        }

        // During countdown, ignore movement so players aren't caught off guard.
        if (countdownActive_)
            return;

        if (state_.gameOver)
        {
            if (k == WXK_RETURN)
            {
                NewGame();
                return;
            }
            if (k == 'R' || k == 'r')
            {
                NewGame();
                return;
            }
            if (k == 'M' || k == 'm')
            {
                GoToMainMenu_();
                return;
            }
            return;
        }

        switch (k)
        {
        case WXK_UP:
            state_.snake.SetDir(Dir::Up);
            break;
        case WXK_DOWN:
            state_.snake.SetDir(Dir::Down);
            break;
        case WXK_LEFT:
            state_.snake.SetDir(Dir::Left);
            break;
        case WXK_RIGHT:
            state_.snake.SetDir(Dir::Right);
            break;
        default:
            break;
        }
    }

    void OnMouseMove(wxMouseEvent &e)
    {
        if (uiMode_ == UiMode::MainMenu)
        {
            if (mainMenu_.IsInteractive())
            {
                mainMenu_.OnMouseMove(e.GetPosition());
                Refresh(false);
            }
            e.Skip();
            return;
        }

        if (uiMode_ == UiMode::TransitionToGame)
        {
            e.Skip();
            return;
        }

        if (pauseMenu_.IsInteractive())
        {
            pauseMenu_.OnMouseMove(e.GetPosition());
            Refresh(false);
        }
        e.Skip();
    }

    void OnMouseDown(wxMouseEvent &e)
    {
        SetFocus();
        SetFocusFromKbd();

        if (uiMode_ == UiMode::MainMenu)
        {
            if (mainMenu_.IsInteractive())
            {
                mainMenu_.OnMouseDown(e.GetPosition());
                Refresh(false);
            }
            e.Skip();
            return;
        }

        if (uiMode_ == UiMode::TransitionToGame)
        {
            e.Skip();
            return;
        }

        // Game over: clickable buttons
        if (uiMode_ == UiMode::Playing && state_.gameOver && !paused_)
        {
            wxPoint p = e.GetPosition();
            if (gameOverRestartBtn_.Contains(p))
            {
                NewGame();
                return;
            }
            if (gameOverMenuBtn_.Contains(p))
            {
                GoToMainMenu_();
                return;
            }
        }

        if (pauseMenu_.IsInteractive())
        {
            pauseMenu_.OnMouseDown(e.GetPosition());
            Refresh(false);
        }
        e.Skip();
    }

    void OnMouseUp(wxMouseEvent &e)
    {
        if (uiMode_ == UiMode::MainMenu)
        {
            if (mainMenu_.IsInteractive())
            {
                mainMenu_.OnMouseUp(e.GetPosition());
                Refresh(false);
            }
            e.Skip();
            return;
        }

        if (uiMode_ == UiMode::TransitionToGame)
        {
            e.Skip();
            return;
        }

        // Game over: clickable buttons
        if (uiMode_ == UiMode::Playing && state_.gameOver && !paused_)
        {
            wxPoint p = e.GetPosition();
            if (gameOverRestartBtn_.Contains(p))
            {
                NewGame();
                return;
            }
            if (gameOverMenuBtn_.Contains(p))
            {
                GoToMainMenu_();
                return;
            }
        }

        if (pauseMenu_.IsInteractive())
        {
            pauseMenu_.OnMouseUp(e.GetPosition());
            Refresh(false);
        }
        e.Skip();
    }

    void OnPaint(wxPaintEvent &)
    {
        wxAutoBufferedPaintDC dc(this);
        dc.Clear();

        wxSize sz = GetClientSize();

        wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
        if (!gc)
            return;

        Skin skin = GetSkin(skinId_);
        const bool lightMode = (skinId_ == SkinId::Light);

        // Base background always
        DrawBackground(gc, wxRect(0, 0, sz.x, sz.y), skin);

        // Common strings
        wxString skinStr = SkinLabel(skinId_);
        wxString soundStr = soundEnabled_ ? "ON" : "OFF";

        // MAIN MENU render
        if (uiMode_ == UiMode::MainMenu)
        {
            mainMenu_.Draw(gc, wxRect(0, 0, sz.x, sz.y), skin, skinId_, skinStr, DifficultyLabel(diff_), soundStr);

            if (skinId_ == SkinId::CRT)
                DrawCrtOverlay(gc, wxRect(0, 0, sz.x, sz.y));

            delete gc;
            return;
        }

        // TRANSITION render (menu behind + bite overlay)
        if (uiMode_ == UiMode::TransitionToGame)
        {
            mainMenu_.Draw(gc, wxRect(0, 0, sz.x, sz.y), skin, skinId_, skinStr, DifficultyLabel(diff_), soundStr);
            DrawBiteOverlay_(gc, wxRect(0, 0, sz.x, sz.y), biteT_);

            if (skinId_ == SkinId::CRT)
                DrawCrtOverlay(gc, wxRect(0, 0, sz.x, sz.y));

            delete gc;
            return;
        }

        // PLAYING DRAW

        const int hudH = 38;
        wxRect hud(0, 0, sz.x, hudH);
        wxRect field(0, hudH, sz.x, sz.y - hudH);

        // grid sizing
        int cellW = field.width / state_.board.w;
        int cellH = field.height / state_.board.h;
        int cell = std::max(10, std::min(cellW, cellH));
        int gridW = cell * state_.board.w;
        int gridH = cell * state_.board.h;

        int ox = field.x + (field.width - gridW) / 2;
        int oy = field.y + (field.height - gridH) / 2;

        wxRect gridRect(ox, oy, gridW, gridH);

        auto cellRect = [&](const Point &p)
        {
            return wxRect(ox + p.x * cell + 2, oy + p.y * cell + 2, cell - 4, cell - 4);
        };

        // field plate
        wxColour fieldPlate = lightMode
                                  ? wxColour(255, 255, 255, 120)
                                  : wxColour(0, 0, 0, 55);

        gc->SetBrush(wxBrush(fieldPlate));
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->DrawRoundedRectangle(ox - 10, oy - 10, gridW + 20, gridH + 20, 14);

        // grid
        if (skinId_ == SkinId::AdvancedGrid)
        {
            DrawAdvancedGrid(gc, gridRect, state_.board.w, state_.board.h, cell, frameCounter_, skin);
        }
        else
        {
            wxColour gridLine;
            if (lightMode)
                gridLine = wxColour(80, 95, 110, 45);
            else if (skinId_ == SkinId::Mono)
                gridLine = wxColour(255, 255, 255, 25);
            else
                gridLine = wxColour(255, 255, 255, 18);

            gc->SetPen(wxPen(gridLine, 1));
            for (int x = 0; x <= state_.board.w; ++x)
                gc->StrokeLine(ox + x * cell, oy, ox + x * cell, oy + gridH);
            for (int y = 0; y <= state_.board.h; ++y)
                gc->StrokeLine(ox, oy + y * cell, ox + gridW, oy + y * cell);
        }

        // gameplay HUD now minimal
        DrawHudScoreOnly(gc, hud, skin, state_.score.value, true, scorePulseTicks_);

        // food
        {
            wxRect fr = cellRect(state_.food.pos);
            wxPoint center(fr.x + fr.width / 2, fr.y + fr.height / 2);

            double pulse = 1.0 + 0.12 * std::sin(frameCounter_ * 0.22);
            int baseRad = std::max(4, fr.width / 4);
            int rad = std::max(3, (int)(baseRad * pulse));

            wxColour core = skin.food;
            wxColour glow = skin.foodGlow;

            if (state_.cfg.foodTtlTicks > 0)
            {
                double tAge = std::min(1.0, state_.foodAgeTicks / (double)state_.cfg.foodTtlTicks);
                int aCore = (int)(255 * (0.75 + 0.25 * (1.0 - tAge)));
                int aGlow = (int)(180 * (0.60 + 0.40 * (1.0 - tAge)));
                core.Set(core.Red(), core.Green(), core.Blue(), aCore);
                glow.Set(glow.Red(), glow.Green(), glow.Blue(), aGlow);
            }

            DrawGlowDot(gc, center, rad, core, glow);
        }

        // enemy snake (Insane)
        if (state_.enemyEnabled && !state_.enemy.body.empty())
        {
            bool eFirst = true;
            wxColour eBody = skin.snakeHead;
            wxColour eHead = lightMode ? wxColour(245, 248, 252) : wxColour(255, 255, 255);

            for (const auto &seg : state_.enemy.body)
            {
                wxRect sr = cellRect(seg);

                wxRect glowR(sr.x - 3, sr.y - 3, sr.width + 6, sr.height + 6);
                wxColour sg = skin.snakeHead;
                sg.Set(sg.Red(), sg.Green(), sg.Blue(), 28);
                DrawRoundedCell(gc, glowR, sg, 8);

                if (eFirst)
                {
                    wxRect headR(sr.x - 1, sr.y - 1, sr.width + 2, sr.height + 2);
                    wxColour hh = eBody;
                    hh.Set(hh.Red(), hh.Green(), hh.Blue(), 220);
                    DrawRoundedCell(gc, headR, hh, 8);
                    DrawRoundedCell(gc, sr, eHead, 7);
                    eFirst = false;
                }
                else
                {
                    wxColour bb = eBody;
                    bb.Set(bb.Red(), bb.Green(), bb.Blue(), 170);
                    DrawRoundedCell(gc, sr, bb, 7);
                }
            }
        }

        // player snake
        bool first = true;
        for (const auto &seg : state_.snake.body)
        {
            wxRect sr = cellRect(seg);

            wxRect glowR(sr.x - 3, sr.y - 3, sr.width + 6, sr.height + 6);
            wxColour sg = skin.snakeGlow;
            sg.Set(sg.Red(), sg.Green(), sg.Blue(), 40);
            DrawRoundedCell(gc, glowR, sg, 8);

            if (first)
            {
                wxRect headR(sr.x - 1, sr.y - 1, sr.width + 2, sr.height + 2);
                DrawRoundedCell(gc, headR, skin.snakeHead, 8);
                first = false;
            }
            else
            {
                DrawRoundedCell(gc, sr, skin.snake, 7);
            }
        }

        // Floating indicator
        if (popLife_ > 0.001f)
        {
            Point lp{popPos_.x, popPos_.y};
            wxRect rr = cellRect(lp);
            wxDouble tw, th, descent, extlead;

            wxFont f = wxFontInfo(18).Family(wxFONTFAMILY_SWISS).Bold().FaceName("Segoe UI");
            int alpha = (int)std::lround(255 * popLife_);
            wxColour txt = skin.text;
            txt.Set(txt.Red(), txt.Green(), txt.Blue(), std::clamp(alpha, 0, 255));
            wxColour sh = lightMode
                              ? wxColour(255, 255, 255, std::clamp((int)std::lround(160 * popLife_), 0, 255))
                              : wxColour(0, 0, 0, std::clamp((int)std::lround(180 * popLife_), 0, 255));

            double rise = (1.0 - popLife_) * 26.0;
            double x = rr.x + rr.width / 2.0;
            double y = rr.y - 10.0 - rise;

            gc->SetFont(f, txt);
            gc->GetTextExtent(popText_, &tw, &th, &descent, &extlead);

            gc->SetFont(f, sh);
            gc->DrawText(popText_, x - tw / 2.0 + 1, y + 1);
            gc->SetFont(f, txt);
            gc->DrawText(popText_, x - tw / 2.0, y);
        }

        if (skinId_ == SkinId::CRT)
            DrawCrtOverlay(gc, wxRect(0, 0, sz.x, sz.y));

        // Countdown overlay
        if (!paused_ && countdownActive_)
        {
            int sec = (countdownMsLeft_ + 999) / 1000;
            if (sec < 1)
                sec = 1;

            wxColour countdownOverlay = lightMode
                                            ? wxColour(255, 255, 255, 150)
                                            : wxColour(0, 0, 0, 150);

            wxColour countdownText = lightMode
                                         ? wxColour(35, 42, 50)
                                         : wxColour(255, 255, 255);

            gc->SetBrush(wxBrush(countdownOverlay));
            gc->SetPen(*wxTRANSPARENT_PEN);
            gc->DrawRoundedRectangle(ox, oy, gridW, gridH, 12);

            wxString num = wxString::Format("%d", sec);
            gc->SetFont(wxFontInfo(72).Bold(), countdownText);
            wxDouble tw, th, descent, extlead;
            gc->GetTextExtent(num, &tw, &th, &descent, &extlead);
            gc->DrawText(num, ox + (gridW - (int)tw) / 2, oy + (gridH - (int)th) / 2);
        }

        if (state_.gameOver)
        {
            wxColour gameOverOverlay = lightMode
                                           ? wxColour(255, 255, 255, 185)
                                           : wxColour(0, 0, 0, 180);

            wxColour gameOverText = lightMode
                                        ? wxColour(35, 42, 50)
                                        : wxColour(255, 255, 255);

            gc->SetBrush(wxBrush(gameOverOverlay));
            gc->SetPen(*wxTRANSPARENT_PEN);
            gc->DrawRoundedRectangle(ox, oy, gridW, gridH, 12);

            gc->SetFont(wxFontInfo(24).Bold(), gameOverText);
            wxString over = "GAME OVER";
            wxDouble tw, th, descent, extlead;
            gc->GetTextExtent(over, &tw, &th, &descent, &extlead);
            gc->DrawText(over, ox + (gridW - (int)tw) / 2, oy + gridH / 2 - 44);

            // Game-over buttons
            const int bw = 180;
            const int bh = 44;
            const int gap = 18;
            int by = oy + gridH / 2 + 6;

            int bx0 = ox + (gridW - (bw * 2 + gap)) / 2;
            int bx1 = bx0 + bw + gap;

            gameOverRestartBtn_ = wxRect(bx0, by, bw, bh);
            gameOverMenuBtn_ = wxRect(bx1, by, bw, bh);

            auto drawBtn = [&](const wxRect &r, const wxString &label)
            {
                wxColour btnFill = lightMode
                                       ? wxColour(255, 255, 255, 210)
                                       : wxColour(0, 0, 0, 170);

                wxColour btnBorder = lightMode
                                         ? wxColour(60, 75, 90, 70)
                                         : wxColour(255, 255, 255, 80);

                wxColour btnText = lightMode
                                       ? wxColour(35, 42, 50)
                                       : wxColour(255, 255, 255);

                gc->SetBrush(wxBrush(btnFill));
                gc->SetPen(wxPen(btnBorder, 1));
                gc->DrawRoundedRectangle(r.x, r.y, r.width, r.height, 12);

                wxFont bf = wxFontInfo(12).Family(wxFONTFAMILY_SWISS).Bold().FaceName("Segoe UI");
                gc->SetFont(bf, btnText);
                wxDouble btw, bth, bd, be;
                gc->GetTextExtent(label, &btw, &bth, &bd, &be);
                gc->DrawText(label, r.x + (r.width - (int)btw) / 2, r.y + (r.height - (int)bth) / 2);
            };

            drawBtn(gameOverRestartBtn_, "Restart");
            drawBtn(gameOverMenuBtn_, "Main Menu");

            // Hint
            wxFont hf = wxFontInfo(11).Family(wxFONTFAMILY_SWISS).Bold().FaceName("Segoe UI");
            wxString hint = "Enter/R: Restart   M: Main Menu";
            wxColour hintText = lightMode
                                    ? wxColour(60, 70, 82)
                                    : wxColour(255, 255, 255);

            gc->SetFont(hf, hintText);
            gc->GetTextExtent(hint, &tw, &th, &descent, &extlead);
            gc->DrawText(hint, ox + (gridW - (int)tw) / 2, by + bh + 10);
        }

        SnakeType stPaint = DetermineTypeFromName(playerName_);
        wxString typeStr = wxString(SnakeTypeName(stPaint));

        wxString diffStr = DifficultyLabel(diff_);
        wxString wrapStr = state_.cfg.wrapWalls ? "Wrap ON" : "Wrap OFF";
        wxString ttlStr = (state_.cfg.foodTtlTicks > 0)
                              ? wxString::Format("Expires (%d ticks)", state_.cfg.foodTtlTicks)
                              : "No Expiry";

        pauseMenu_.Draw(gc, wxRect(0, 0, sz.x, sz.y), skin,
                        state_.score.value,
                        playerName_,
                        typeStr,
                        skinStr,
                        diffStr,
                        wrapStr,
                        ttlStr,
                        soundStr);

        delete gc;
    }
};

wxPanel *CreateGamePanel(wxWindow *parent)
{
    return new GamePanel(parent);
}
