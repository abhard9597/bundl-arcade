#pragma once
#include <wx/wx.h>

/**
 * @brief Provides theme state and colour palette utilities for the application UI.
 *
 * This namespace groups the palette definition and helper functions used to retrieve the current application theme styling.
 * It is intended to centralize colour choices for backgrounds, cards, text, inputs, buttons, overlays, and shadows.
 *
 * @author Aditi Bhardwaj
 */
namespace AppTheme
{
    /**
     * @brief Stores the full set of colours used by the application theme.
     *
     * This structure bundles all colours needed to style the interface consistently across backgrounds, cards, text, inputs, buttons, and overlays.
     */
    struct Palette
    {
        /** @brief Background colour for the main application area. */
        wxColour bg;

        /** @brief Fill colour for card-style containers. */
        wxColour card;

        /** @brief Border colour used around cards. */
        wxColour cardEdge;

        /** @brief Primary text colour. */
        wxColour text;

        /** @brief Secondary or muted text colour. */
        wxColour subtle;

        /** @brief Background colour for input controls. */
        wxColour inputBg;

        /** @brief Foreground or text colour for input controls. */
        wxColour inputFg;

        /** @brief Default button fill colour. */
        wxColour button;

        /** @brief Button fill colour when hovered. */
        wxColour buttonHover;

        /** @brief Button fill colour when pressed. */
        wxColour buttonPress;

        /** @brief Overlay colour used for layered UI effects. */
        wxColour overlay;

        /** @brief Shadow colour used for depth effects. */
        wxColour shadow;
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
     * @return A Palette containing all colours for the current application theme.
     */
    Palette GetPalette();
}