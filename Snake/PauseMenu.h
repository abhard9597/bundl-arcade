/**
 * @file PauseMenu.h
 * @brief Pause menu declarations for the snake game.
 *
 * This file declares the PauseMenu class, which manages the in-game
 * pause overlay, navigation pages, buttons, options, and drawing logic.
 *
 * @author Atabong Eyambe Mokom
 */
#pragma once
#include <wx/wx.h>
#include <wx/graphics.h>
#include <wx/utils.h>
#include <vector>
#include <functional>
#include "Theme.h"

/**
 * @brief Represents the in-game pause menu.
 *
 * The PauseMenu class provides the pause overlay interface, including
 * its pages, buttons, scrollable content, input handling, animation,
 * and themed rendering.
 *
 * @author Atabong Eyambe Mokom
 */
class PauseMenu
{
public:
    /**
     * @brief Represents the pages available in the pause menu.
     *
     * @author Atabong Eyambe Mokom
     */
    enum class Page
    {
        Main,
        Options,
        Tutorial
    };

    /**
     * @brief Represents a clickable pause menu button.
     *
     * @author Atabong Eyambe Mokom
     */
    struct Button
    {
        wxString label;                ///< Button text.
        std::function<void()> onClick; ///< Callback executed when the button is clicked.

        wxRect rect;           ///< Rectangle occupied by the button.
        bool hovered = false;  ///< Whether the button is hovered.
        float hoverAnim = 0.f; ///< Hover animation progress.

        bool pressed = false;  ///< Whether the button is pressed.
        float pressAnim = 0.f; ///< Press animation progress.
    };

    /**
     * @brief Sets callbacks for the main pause menu actions.
     *
     * @param resume Callback for resuming the game.
     * @param restart Callback for restarting the game.
     * @param stats Callback for opening statistics.
     * @param controls Callback for opening controls.
     * @param tutorial Callback for opening the tutorial page.
     * @param mainMenu Callback for returning to the main menu.
     *
     * @author Atabong Eyambe Mokom
     */
    void SetActions(std::function<void()> resume,
                    std::function<void()> restart,
                    std::function<void()> stats,
                    std::function<void()> controls,
                    std::function<void()> tutorial,
                    std::function<void()> mainMenu);

    /**
     * @brief Sets callbacks for pause menu options.
     *
     * @param toggleWrap Callback for toggling wall wrap mode.
     * @param cycleDifficulty Callback for cycling difficulty.
     * @param cycleSkin Callback for cycling skins.
     * @param toggleSound Callback for toggling sound.
     *
     * @author Atabong Eyambe Mokom
     */
    void SetOptions(std::function<void()> toggleWrap,
                    std::function<void()> cycleDifficulty,
                    std::function<void()> cycleSkin,
                    std::function<void()> toggleSound);

    /**
     * @brief Sets sound effect callbacks for hover and click actions.
     *
     * @param onHover Callback triggered on hover.
     * @param onClick Callback triggered on click.
     *
     * @author Atabong Eyambe Mokom
     */
    void SetSfxCallbacks(std::function<void()> onHover,
                         std::function<void()> onClick);

    /**
     * @brief Opens the pause menu.
     *
     * @author Atabong Eyambe Mokom
     */
    void Open();

    /**
     * @brief Closes the pause menu.
     *
     * @author Atabong Eyambe Mokom
     */
    void Close();

    /**
     * @brief Checks whether the pause menu is open.
     *
     * @return true if the pause menu is open.
     * @return false otherwise.
     *
     * @author Atabong Eyambe Mokom
     */
    bool IsOpen() const;

    /**
     * @brief Returns the current pause menu animation value.
     *
     * @return The current animation progress.
     *
     * @author Atabong Eyambe Mokom
     */
    float Anim() const;

    /**
     * @brief Checks whether the pause menu is interactive.
     *
     * @return true if the menu can receive input.
     * @return false otherwise.
     *
     * @author Atabong Eyambe Mokom
     */
    bool IsInteractive() const;

    /**
     * @brief Sets the current pause menu page.
     *
     * @param p The page to show.
     *
     * @author Atabong Eyambe Mokom
     */
    void SetPage(Page p);

    /**
     * @brief Returns the current pause menu page.
     *
     * @return The active page.
     *
     * @author Atabong Eyambe Mokom
     */
    Page GetPage() const;

    /**
     * @brief Updates the pause menu animation and state.
     *
     * @author Atabong Eyambe Mokom
     */
    void Tick();

    /**
     * @brief Handles mouse movement over the pause menu.
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
     * @brief Handles keyboard navigation input.
     *
     * @param keyCode The key code that was pressed.
     *
     * @author Atabong Eyambe Mokom
     */
    void OnKeyDown(int keyCode);

    /**
     * @brief Draws the pause menu.
     *
     * @param gc The graphics context used for drawing.
     * @param screen The screen drawing rectangle.
     * @param skin The active skin.
     * @param score The current player score.
     * @param playerName The player name.
     * @param typeStr The current snake type label.
     * @param skinStr The current skin label.
     * @param diffStr The current difficulty label.
     * @param wrapStr The current wall-wrap label.
     * @param ttlStr The title text to display.
     * @param soundStr The current sound setting label.
     *
     * @author Atabong Eyambe Mokom
     */
    void Draw(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin,
              int score,
              const wxString &playerName,
              const wxString &typeStr,
              const wxString &skinStr,
              const wxString &diffStr,
              const wxString &wrapStr,
              const wxString &ttlStr,
              const wxString &soundStr);

private:
    Page page_ = Page::Main; ///< Current pause menu page.

    float anim_ = 0.f;   ///< Current animation value.
    float target_ = 0.f; ///< Target animation value.

    wxPoint mouse_{0, 0}; ///< Current mouse position.

    std::vector<Button> buttons_;       ///< Pause menu buttons.
    std::function<void()> actResume_;   ///< Resume callback.
    std::function<void()> actRestart_;  ///< Restart callback.
    std::function<void()> actStats_;    ///< Statistics callback.
    std::function<void()> actControls_; ///< Controls callback.
    std::function<void()> actTutorial_; ///< Tutorial callback.
    std::function<void()> actMainMenu_; ///< Main menu callback.

    std::function<void()> optToggleWrap_;      ///< Wrap toggle callback.
    std::function<void()> optCycleDifficulty_; ///< Difficulty cycle callback.
    std::function<void()> optCycleSkin_;       ///< Skin cycle callback.
    std::function<void()> optToggleSound_;     ///< Sound toggle callback.

    std::function<void()> sfxHover_; ///< Hover sound callback.
    std::function<void()> sfxClick_; ///< Click sound callback.

    double stripPhase_ = 0.0;  ///< Decorative strip animation phase.
    long long lastAnimMs_ = 0; ///< Last animation update time.

    wxRect lastLayoutScreen_{};        ///< Cached layout screen rectangle.
    Page lastLayoutPage_ = Page::Main; ///< Last layout page.

    int scrollOffset_ = 0;   ///< Current scroll offset.
    int scrollTarget_ = 0;   ///< Target scroll offset.
    int visibleTopY_ = 0;    ///< Visible top boundary.
    int visibleBottomY_ = 0; ///< Visible bottom boundary.
    int contentBottomY_ = 0; ///< Content bottom boundary.

    int selectedIndex_ = 0; ///< Currently selected button index.

    /**
     * @brief Sets the selected button index.
     *
     * @param idx The new selected index.
     *
     * @author Atabong Eyambe Mokom
     */
    void SetSelected_(int idx);

    /**
     * @brief Ensures a selected item remains visible in the scroll area.
     *
     * @param idx The index of the selected button.
     *
     * @author Atabong Eyambe Mokom
     */
    void EnsureVisible_(int idx);

    /**
     * @brief Rebuilds button layout for a new screen rectangle.
     *
     * @param screen The current screen rectangle.
     *
     * @author Atabong Eyambe Mokom
     */
    void RebuildButtons(const wxRect &screen);

    /**
     * @brief Updates button hover state.
     *
     * @author Atabong Eyambe Mokom
     */
    void UpdateHover();

    /**
     * @brief Clamps scroll values to valid limits.
     *
     * @author Atabong Eyambe Mokom
     */
    void ClampScroll_();

    /**
     * @brief Returns the animated rectangle for a button.
     *
     * @param index The button index.
     * @return The animated button rectangle.
     *
     * @author Atabong Eyambe Mokom
     */
    wxRect AnimatedRectFor(int index) const;

    /**
     * @brief Draws the dim overlay behind the pause menu.
     *
     * @param gc The graphics context used for drawing.
     * @param screen The screen rectangle.
     * @param a The alpha or intensity value.
     *
     * @author Atabong Eyambe Mokom
     */
    void DrawDim(wxGraphicsContext *gc, const wxRect &screen, float a, bool lightMode);

    /**
     * @brief Draws the animated decorative snake strip.
     *
     * @param gc The graphics context used for drawing.
     * @param screen The screen rectangle.
     * @param skin The active skin.
     * @param y The vertical position for the strip.
     * @param leftToRight Whether the strip moves left-to-right.
     *
     * @author Atabong Eyambe Mokom
     */
    void DrawSnakeStrip(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin, int y, bool leftToRight);

    /**
     * @brief Draws the main pause page.
     *
     * @param gc The graphics context used for drawing.
     * @param screen The screen rectangle.
     * @param skin The active skin.
     * @param score The current score.
     * @param playerName The player name.
     * @param typeStr The snake type label.
     * @param skinStr The skin label.
     * @param diffStr The difficulty label.
     * @param wrapStr The wrap setting label.
     * @param ttlStr The title text.
     *
     * @author Atabong Eyambe Mokom
     */
    void DrawMain(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin,
                  int score,
                  const wxString &playerName,
                  const wxString &typeStr,
                  const wxString &skinStr,
                  const wxString &diffStr,
                  const wxString &wrapStr,
                  const wxString &ttlStr,
                  bool lightMode);

    /**
     * @brief Draws the options page.
     *
     * @param gc The graphics context used for drawing.
     * @param screen The screen rectangle.
     * @param skin The active skin.
     * @param skinStr The skin label.
     * @param diffStr The difficulty label.
     * @param wrapStr The wrap setting label.
     * @param soundStr The sound setting label.
     *
     * @author Atabong Eyambe Mokom
     */
    void DrawOptions(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin,
                     const wxString &skinStr,
                     const wxString &diffStr,
                     const wxString &wrapStr,
                     const wxString &soundStr,
                     bool lightMode);

    /**
     * @brief Draws the tutorial page.
     *
     * @param gc The graphics context used for drawing.
     * @param screen The screen rectangle.
     * @param skin The active skin.
     *
     * @author Atabong Eyambe Mokom
     */
    void DrawTutorial(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin, bool lightMode);

    /**
     * @brief Draws a pause menu button.
     *
     * @param gc The graphics context used for drawing.
     * @param b The button to draw.
     * @param skin The active skin.
     * @param index The button index.
     *
     * @author Atabong Eyambe Mokom
     */
    void DrawButton(wxGraphicsContext *gc, Button &b, const Skin &skin, int index, bool lightMode);

    /**
     * @brief Applies an ease-out interpolation function.
     *
     * @param t The interpolation parameter.
     * @return The eased interpolation value.
     *
     * @author Atabong Eyambe Mokom
     */
    static float EaseOut(float t);
};