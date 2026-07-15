#ifndef CONNECTFOUR_APP_THEME_H
#define CONNECTFOUR_APP_THEME_H

#include <wx/wx.h>

/**
 * @brief Provides theme utilities and colour palette data for Connect Four.
 *
 * This namespace contains the palette structure and helper functions used todetermine the current theme
 * and retrieve the colours used throughout the Connect Four interface.
 *
 * @author Aditi Bhardwaj
 */
namespace AppTheme
{
    /**
     * @brief Stores the set of colours used by the Connect Four theme.
     *
     * This structure groups the colours needed to style the frame, panels, cards, text, accents, shadows, and divider lines consistently.
     */
    struct Palette
    {
        /** @brief Background colour for the main frame. */
        wxColour frameBg;

        /** @brief Background colour for the main panel area. */
        wxColour panelBg;

        /** @brief Default colour for card-style elements. */
        wxColour card;

        /** @brief Card colour when hovered. */
        wxColour cardHover;

        /** @brief Card colour when pressed. */
        wxColour cardPress;

        /** @brief Primary text colour. */
        wxColour text;

        /** @brief Secondary or muted text colour. */
        wxColour subtleText;

        /** @brief Accent colour used for highlighted UI elements. */
        wxColour accent;

        /** @brief Shadow colour used for depth effects. */
        wxColour shadow;

        /** @brief Line or border colour used for separators and edges. */
        wxColour line;
    };

    /**
     * @brief Returns whether the application is currently using dark mode.
     *
     * @return True if dark mode is enabled, otherwise false.
     */
    bool IsDarkMode();

    /**
     * @brief Returns the active colour palette for the current theme.
     *
     * @return A Palette containing the colours for the current Connect Four theme.
     */
    Palette GetPalette();
}

#endif