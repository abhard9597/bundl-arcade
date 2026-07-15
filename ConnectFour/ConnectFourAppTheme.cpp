#include "ConnectFourAppTheme.h"
#include <wx/utils.h>

namespace AppTheme
{
    /**
     * @brief Returns whether dark mode is currently enabled.
     *
     * This function checks the BUNDL_DARK_MODE environment variable to determine whether the Connect Four interface should use the dark theme.
     * If the variable is not set, dark mode is enabled by default.
     *
     * @return True if dark mode is enabled, otherwise false.
     */
    bool IsDarkMode()
    {
        wxString value;
        if (!wxGetEnv("BUNDL_DARK_MODE", &value))
            return true; // default dark

        return value == "1" || value.CmpNoCase("true") == 0;
    }

    /**
     * @brief Returns the active colour palette for the current theme.
     *
     * This function builds and returns a Palette containing the colours used throughout the Connect Four interface based on whether dark mode is enabled.
     *
     * @return A Palette containing the colours for the current Connect Four theme.
     */
    Palette GetPalette()
    {
        if (IsDarkMode())
        {
            return {
                wxColour(18, 24, 32),       // frameBg
                wxColour(18, 24, 32),       // panelBg
                wxColour(16, 26, 38, 220),  // card
                wxColour(22, 36, 52, 235),  // cardHover
                wxColour(12, 20, 30, 245),  // cardPress
                wxColour(245, 250, 255),    // text
                wxColour(190, 205, 220),    // subtleText
                wxColour(170, 210, 230),    // accent
                wxColour(0, 0, 0, 85),      // shadow
                wxColour(255, 255, 255, 30) // line
            };
        }

        return {
            wxColour(241, 245, 249),      // frameBg
            wxColour(241, 245, 249),      // panelBg
            wxColour(255, 255, 255, 235), // card
            wxColour(244, 248, 252, 245), // cardHover
            wxColour(232, 238, 244, 245), // cardPress
            wxColour(32, 41, 52),         // text
            wxColour(84, 96, 110),        // subtleText
            wxColour(91, 155, 213),       // accent
            wxColour(0, 0, 0, 30),        // shadow
            wxColour(0, 0, 0, 22)         // line
        };
    }
}