#include "AppTheme.h"
#include <wx/utils.h>

namespace AppTheme
{
    /**
     * @brief Returns whether dark mode is currently enabled.
     *
     * This function checks the BUNDL_DARK_MODE environment variable to determine whether the application should use the dark theme.
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
     * This function builds and returns a Palette containing all colours used by the interface based on whether dark mode is enabled.
     *
     * @return A Palette containing the colours for the current theme.
     */
    Palette GetPalette()
    {
        if (IsDarkMode())
        {
            return {
                wxColour(18, 24, 32),        // bg
                wxColour(22, 32, 44),        // card
                wxColour(255, 255, 255, 28), // cardEdge
                wxColour(245, 250, 255),     // text
                wxColour(190, 205, 220),     // subtle
                wxColour(16, 24, 34),        // inputBg
                wxColour(245, 250, 255),     // inputFg
                wxColour(44, 70, 92),        // button
                wxColour(58, 92, 118),       // buttonHover
                wxColour(36, 58, 76),        // buttonPress
                wxColour(0, 0, 0, 55),       // overlay
                wxColour(0, 0, 0, 65)        // shadow
            };
        }

        return {
            wxColour(242, 246, 250),     // bg
            wxColour(255, 255, 255),     // card
            wxColour(0, 0, 0, 20),       // cardEdge
            wxColour(35, 42, 50),        // text
            wxColour(90, 103, 117),      // subtle
            wxColour(248, 250, 252),     // inputBg
            wxColour(35, 42, 50),        // inputFg
            wxColour(206, 222, 236),     // button
            wxColour(188, 210, 228),     // buttonHover
            wxColour(170, 195, 216),     // buttonPress
            wxColour(255, 255, 255, 35), // overlay
            wxColour(0, 0, 0, 18)        // shadow
        };
    }
}
