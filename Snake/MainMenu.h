/**
 * @file MainMenu.h
 * @brief Main menu declarations for the snake game.
 *
 * This file declares the MainMenu class, which manages the animated
 * start menu, its buttons, options, and decorative visual effects.
 *
 * @author Atabong Eyambe Mokom
 */
#pragma once
#include <wx/wx.h>
#include <wx/graphics.h>
#include <wx/utils.h>
#include <vector>
#include <functional>
#include <algorithm>
#include <cmath>
#include "Theme.h"

/**
 * @brief Represents the main menu of the snake game.
 *
 * The MainMenu class manages user interaction, animations, option
 * handling, and themed rendering for the game's main menu screen.
 *
 * @author Atabong Eyambe Mokom
 */
class MainMenu
{
public:
    /**
     * @brief Represents a clickable menu button.
     *
     * A Button stores its label, click callback, geometry, and
     * animation state for hover and press effects.
     *
     * @author Atabong Eyambe Mokom
     */
    struct Button
    {
        wxString label;                ///< Button text.
        std::function<void()> onClick; ///< Callback executed when the button is clicked.

        wxRect rect;           ///< Rectangle occupied by the button.
        bool hovered = false;  ///< Whether the mouse is over the button.
        float hoverAnim = 0.f; ///< Hover animation progress.

        bool pressed = false;  ///< Whether the button is being pressed.
        float pressAnim = 0.f; ///< Press animation progress.
    };

    /**
     * @brief Sets the callbacks for the main menu actions.
     *
     * @param startGame Callback for starting the game.
     * @param stats Callback for opening the statistics screen.
     * @param tutorials Callback for opening the tutorials screen.
     * @param exitGame Callback for exiting the game.
     *
     * @author Atabong Eyambe Mokom
     */
    void SetActions(std::function<void()> startGame,
                    std::function<void()> stats,
                    std::function<void()> tutorials,
                    std::function<void()> exitGame);

    /**
     * @brief Sets the callbacks for menu options.
     *
     * @param cycleSkin Callback for cycling the current skin.
     * @param cycleDifficulty Callback for cycling the difficulty.
     * @param toggleSound Callback for toggling sound on or off.
     *
     * @author Atabong Eyambe Mokom
     */
    void SetOptions(std::function<void()> cycleSkin,
                    std::function<void()> cycleDifficulty,
                    std::function<void()> toggleSound);

    /**
     * @brief Sets sound effect callbacks for hover and click actions.
     *
     * @param onHover Callback triggered when hovering over a button.
     * @param onClick Callback triggered when clicking a button.
     *
     * @author Atabong Eyambe Mokom
     */
    void SetSfxCallbacks(std::function<void()> onHover,
                         std::function<void()> onClick);

    /**
     * @brief Opens the main menu.
     *
     * @author Atabong Eyambe Mokom
     */
    void Open();

    /**
     * @brief Closes the main menu.
     *
     * @author Atabong Eyambe Mokom
     */
    void Close();

    /**
     * @brief Checks whether the main menu is open.
     *
     * @return true if the menu is open.
     * @return false otherwise.
     *
     * @author Atabong Eyambe Mokom
     */
    bool IsOpen() const;

    /**
     * @brief Returns the current animation progress of the main menu.
     *
     * @return The current animation value.
     *
     * @author Atabong Eyambe Mokom
     */
    float Anim() const;

    /**
     * @brief Checks whether the main menu is currently interactive.
     *
     * @return true if the menu can receive input.
     * @return false otherwise.
     *
     * @author Atabong Eyambe Mokom
     */
    bool IsInteractive() const;

    /**
     * @brief Updates the main menu animation state.
     *
     * @author Atabong Eyambe Mokom
     */
    void Tick();

    /**
     * @brief Handles mouse movement over the main menu.
     *
     * @param p The current mouse position.
     *
     * @author Atabong Eyambe Mokom
     */
    void OnMouseMove(const wxPoint &p);

    /**
     * @brief Handles mouse button press input.
     *
     * @param p The mouse position at the time of the press.
     *
     * @author Atabong Eyambe Mokom
     */
    void OnMouseDown(const wxPoint &p);

    /**
     * @brief Handles mouse button release input.
     *
     * @param p The mouse position at the time of the release.
     *
     * @author Atabong Eyambe Mokom
     */
    void OnMouseUp(const wxPoint &p);

    /**
     * @brief Handles keyboard input for menu navigation.
     *
     * @param keyCode The code of the pressed key.
     *
     * @author Atabong Eyambe Mokom
     */
    void OnKeyDown(int keyCode);

    /**
     * @brief Draws the main menu.
     *
     * This function renders the main menu using the current theme
     * and the provided option labels.
     *
     * @param gc The graphics context used for drawing.
     * @param screen The drawing area of the screen.
     * @param skin The active skin.
     * @param skinId The current skin identifier.
     * @param skinStr The label for the current skin.
     * @param diffStr The label for the current difficulty.
     * @param soundStr The label for the current sound setting.
     *
     * @author Atabong Eyambe Mokom
     */
    void Draw(wxGraphicsContext *gc,
              const wxRect &screen,
              const Skin &skin,
              SkinId skinId,
              const wxString &skinStr,
              const wxString &difficultyStr,
              const wxString &soundStr);

private:
    float anim_ = 0.f;   ///< Current animation progress.
    float target_ = 0.f; ///< Target animation progress.

    wxPoint mouse_{0, 0}; ///< Current mouse position.

    std::vector<Button> buttons_; ///< Menu button list.
    int selectedIndex_ = 0;       ///< Currently selected button index.

    std::function<void()> actStart_;     ///< Start game action callback.
    std::function<void()> actStats_;     ///< Statistics action callback.
    std::function<void()> actTutorials_; ///< Tutorials action callback.
    std::function<void()> actExit_;      ///< Exit action callback.

    std::function<void()> optCycleSkin_;       ///< Skin option callback.
    std::function<void()> optCycleDifficulty_; ///< Difficulty option callback.
    std::function<void()> optToggleSound_;     ///< Sound option callback.

    std::function<void()> sfxHover_; ///< Hover sound callback.
    std::function<void()> sfxClick_; ///< Click sound callback.

    double phase_ = 0.0;   ///< Animation phase timer.
    long long lastMs_ = 0; ///< Last animation update time.

    wxRect lastLayoutScreen_{}; ///< Cached layout screen rectangle.

    wxString curSkinStr_;  ///< Current skin label.
    wxString curDiffStr_;  ///< Current difficulty label.
    wxString curSoundStr_; ///< Current sound label.

    /**
     * @brief Represents a star or shard in the animated background.
     *
     * @author Atabong Eyambe Mokom
     */
    struct Star
    {
        float x = 0.f;     ///< X-coordinate.
        float y = 0.f;     ///< Y-coordinate.
        float speed = 0.f; ///< Movement speed.
        float size = 1.f;  ///< Star size.
        float tw = 0.f;    ///< Twinkle or animation value.
        float vx = 0.f;    ///< Horizontal drift velocity.
        float vy = 0.f;    ///< Vertical drift velocity.
    };

    std::vector<Star> stars_; ///< Background star list.
    bool starsInit_ = false;  ///< Whether stars have been initialized.

    /**
     * @brief Applies an ease-out interpolation function.
     *
     * @param t The interpolation parameter.
     * @return The eased value.
     *
     * @author Atabong Eyambe Mokom
     */
    static float EaseOut(float t);

    /**
     * @brief Rebuilds menu buttons for a new screen size.
     *
     * @param screen The screen rectangle.
     *
     * @author Atabong Eyambe Mokom
     */
    void RebuildButtons_(const wxRect &screen);

    /**
     * @brief Updates which button is currently hovered.
     *
     * @author Atabong Eyambe Mokom
     */
    void UpdateHover_();

    /**
     * @brief Sets the selected button index.
     *
     * @param idx The new selected index.
     *
     * @author Atabong Eyambe Mokom
     */
    void SetSelected_(int idx);

    /**
     * @brief Returns the animated rectangle for a button.
     *
     * @param index The index of the button.
     * @return The animated button rectangle.
     *
     * @author Atabong Eyambe Mokom
     */
    wxRect AnimatedRectFor_(int index) const;

    /**
     * @brief Ensures the starfield is initialized.
     *
     * @param screen The screen rectangle.
     * @param personaMode Whether persona mode visuals are enabled.
     *
     * @author Atabong Eyambe Mokom
     */
    void EnsureStars_(const wxRect &screen, bool personaMode);

    /**
     * @brief Updates star animation.
     *
     * @param screen The screen rectangle.
     * @param dt Delta time since the last update.
     * @param personaMode Whether persona mode visuals are enabled.
     *
     * @author Atabong Eyambe Mokom
     */
    void TickStars_(const wxRect &screen, float dt, bool personaMode);

    /**
     * @brief Draws the animated starfield background.
     *
     * @param gc The graphics context used for drawing.
     * @param screen The screen rectangle.
     * @param skin The active skin.
     * @param t The animation time value.
     * @param personaMode Whether persona mode visuals are enabled.
     *
     * @author Atabong Eyambe Mokom
     */
    void DrawStarfield_(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin, float t, bool personaMode);

    /**
     * @brief Draws the void snake decorative effect.
     *
     * @param gc The graphics context used for drawing.
     * @param screen The screen rectangle.
     * @param skin The active skin.
     * @param t The animation time value.
     * @param personaMode Whether persona mode visuals are enabled.
     *
     * @author Atabong Eyambe Mokom
     */
    void DrawVoidSnake_(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin, float t, bool personaMode);

    /**
     * @brief Draws the stylized title.
     *
     * @param gc The graphics context used for drawing.
     * @param screen The screen rectangle.
     * @param skin The active skin.
     * @param t The animation time value.
     * @param personaMode Whether persona mode visuals are enabled.
     *
     * @author Atabong Eyambe Mokom
     */
    void DrawTitle2Bit_(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin, float t, bool personaMode);

    /**
     * @brief Draws a menu button.
     *
     * @param gc The graphics context used for drawing.
     * @param b The button to draw.
     * @param skin The active skin.
     * @param index The index of the button.
     *
     * @author Atabong Eyambe Mokom
     */
    void DrawButton_(wxGraphicsContext *gc, Button &b, const Skin &skin, SkinId skinId, int index);

    /**
     * @brief Returns a copy of a color with a specified alpha value.
     *
     * @param c The base color.
     * @param a The new alpha value.
     * @return A copy of the color with the specified alpha.
     *
     * @author Atabong Eyambe Mokom
     */
    static wxColour WithAlpha_(wxColour c, int a)
    {
        c.Set(c.Red(), c.Green(), c.Blue(), std::clamp(a, 0, 255));
        return c;
    }
};