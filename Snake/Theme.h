/**
 * @file Theme.h
 * @brief Theme, skin, and drawing helper declarations for the snake game.
 *
 * This file defines the available skins and drawing helper functions
 * used to render the game background, cells, glow effects, CRT overlay,
 * HUD, and advanced grid visuals.
 *
 * @author Atabong Eyambe Mokom
 */
#pragma once
#include <wx/wx.h>
#include <wx/graphics.h>

/**
 * @brief Represents the available visual skin identifiers.
 *
 * Each SkinId corresponds to a different style preset for the game.
 *
 * @author Atabong Eyambe Mokom
 */
enum class SkinId
{
    Classic,
    Light,
    Neon,
    Amber,
    Mono,
    CRT,
    AdvancedGrid
};

/**
 * @brief Stores the visual appearance settings for a skin.
 *
 * A Skin contains the color palette and fonts used to draw the game.
 *
 * @author Atabong Eyambe Mokom
 */
struct Skin
{
    wxColour bg1, bg2; ///< Primary background colors.
    wxColour hudBg;    ///< HUD background color.
    wxColour text;     ///< Text color.

    wxColour snake;     ///< Snake body color.
    wxColour snakeHead; ///< Snake head color.
    wxColour snakeGlow; ///< Snake glow color.

    wxColour food;     ///< Food color.
    wxColour foodGlow; ///< Food glow color.

    wxFont hudFontSmall; ///< Small HUD or menu font.
    wxFont hudFontBig;   ///< Large HUD or menu font.
};

/**
 * @brief Returns the skin data for a given skin identifier.
 *
 * @param id The selected skin identifier.
 * @return The Skin structure corresponding to the selected theme.
 *
 * @author Atabong Eyambe Mokom
 */
Skin GetSkin(SkinId id);

/**
 * @brief Draws the background of a region.
 *
 * @param gc The graphics context used for drawing.
 * @param r The rectangle defining the drawing area.
 * @param s The skin used for the background.
 *
 * @author Atabong Eyambe Mokom
 */
void DrawBackground(wxGraphicsContext *gc, const wxRect &r, const Skin &s);

/**
 * @brief Draws a rounded rectangular cell.
 *
 * @param gc The graphics context used for drawing.
 * @param r The rectangle defining the cell bounds.
 * @param fill The fill color of the cell.
 * @param radius The radius of the rounded corners.
 *
 * @author Atabong Eyambe Mokom
 */
void DrawRoundedCell(wxGraphicsContext *gc, const wxRect &r, const wxColour &fill, int radius);

/**
 * @brief Draws a glowing dot effect.
 *
 * @param gc The graphics context used for drawing.
 * @param c The center point of the dot.
 * @param rad The radius of the core dot.
 * @param core The main inner color.
 * @param glow The outer glow color.
 *
 * @author Atabong Eyambe Mokom
 */
void DrawGlowDot(wxGraphicsContext *gc, const wxPoint &c, int rad, const wxColour &core, const wxColour &glow);

/**
 * @brief Draws a CRT overlay effect.
 *
 * @param gc The graphics context used for drawing.
 * @param r The rectangle defining the area to overlay.
 *
 * @author Atabong Eyambe Mokom
 */
void DrawCrtOverlay(wxGraphicsContext *gc, const wxRect &r);

/**
 * @brief Draws the gameplay HUD showing only score-related information.
 *
 * @param gc The graphics context used for drawing.
 * @param r The rectangle defining the HUD area.
 * @param s The skin used for drawing.
 * @param score The current score.
 * @param showHint Whether a hint should be displayed.
 * @param scorePulseTicks Remaining score pulse animation ticks.
 *
 * @author Atabong Eyambe Mokom
 */
void DrawHudScoreOnly(wxGraphicsContext *gc, const wxRect &r, const Skin &s,
                      int score, bool showHint, int scorePulseTicks);

/**
 * @brief Draws the animated advanced grid effect.
 *
 * @param gc The graphics context used for drawing.
 * @param gridRect The rectangle containing the grid.
 * @param cols The number of columns.
 * @param rows The number of rows.
 * @param cell The cell size in pixels.
 * @param frameCounter The current animation frame count.
 * @param s The skin used for the grid.
 *
 * @author Atabong Eyambe Mokom
 */
void DrawAdvancedGrid(wxGraphicsContext *gc, const wxRect &gridRect,
                      int cols, int rows, int cell, int frameCounter,
                      const Skin &s);