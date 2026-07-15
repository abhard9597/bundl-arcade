#ifndef UIHELPERS_H
#define UIHELPERS_H

#include <wx/wx.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/filefn.h>
#include <wx/image.h>
#include <wx/sound.h>
#include <algorithm>

/**
 * @brief Utility helpers and shared UI constants for the bundl. launcher.
 *
 * This namespace contains reusable values and helper functions used to style, paint, and support the launcher interface.
 * It centralizes colours, panel sizing, asset path lookup, sound playback, and shared drawing logic so the menu and settings screens stay visually consistent.
 *
 * @author Aditi Bhardwaj
 */
namespace UiHelpers
{
    /** @brief Standard width used for the centered content panel. */
    inline constexpr int panelWidth = 360;

    /** @brief Standard height used for the menu panel glass card. */
    inline constexpr int menuPanelHeight = 437;

    /** @brief Standard height used for the settings panel glass card. */
    inline constexpr int settingsPanelHeight = 345;

    /** @brief Standard width used for buttons and toggle rows. */
    inline constexpr int controlWidth = 280;

    /** @brief Standard height used for buttons and toggle rows. */
    inline constexpr int controlHeight = 50;

    /** @brief Standard vertical spacing between controls. */
    inline constexpr int controlGap = 10;

    /** @brief Standard corner radius used for glass panels. */
    inline constexpr double panelRadius = 18.0;

    /**
     * @brief Stores the visual state for a painted menu button.
     *
     * This structure is used by the menu panel when buttons are drawn directly
     * onto the panel instead of being separate child controls.
     */
    struct PaintedButtonState
    {
        /** @brief Whether the lighter visual style should be used. */
        bool lighterStyle = false;

        /** @brief Whether the painted button is currently hovered. */
        bool hovered = false;

        /** @brief Whether the painted button is currently pressed. */
        bool pressed = false;
    };

    /**
     * @brief Returns the frame background colour for the active theme.
     *
     * @param darkMode True for dark mode, false for light mode.
     * @return The background colour for the main frame.
     */
    inline wxColour FrameBackground(bool darkMode)
    {
        return darkMode ? wxColour(8, 10, 14) : wxColour(234, 239, 244);
    }

    /**
     * @brief Returns the translucent overlay colour for the active theme.
     *
     * @param darkMode True for dark mode, false for light mode.
     * @return The overlay colour used above the background image.
     */
    inline wxColour OverlayColour(bool darkMode)
    {
        return darkMode ? wxColour(5, 8, 12, 130) : wxColour(232, 238, 244, 58);
    }

    /**
     * @brief Returns the glass panel fill colour for the active theme.
     *
     * @param darkMode True for dark mode, false for light mode.
     * @return The fill colour used for glass-style panels.
     */
    inline wxColour PanelFill(bool darkMode)
    {
        return darkMode ? wxColour(8, 13, 19, 165) : wxColour(240, 244, 248, 172);
    }

    /**
     * @brief Returns the border colour for glass panels.
     *
     * @param darkMode True for dark mode, false for light mode.
     * @return The border colour used around the main panels.
     */
    inline wxColour PanelBorder(bool darkMode)
    {
        return darkMode ? wxColour(88, 102, 118, 150) : wxColour(155, 165, 175, 135);
    }

    /**
     * @brief Returns the primary text colour for the active theme.
     *
     * @param darkMode True for dark mode, false for light mode.
     * @return The main text colour.
     */
    inline wxColour TextPrimary(bool darkMode)
    {
        return darkMode ? wxColour(236, 241, 245) : wxColour(28, 34, 40);
    }

    /**
     * @brief Returns the secondary text colour for the active theme.
     *
     * @param darkMode True for dark mode, false for light mode.
     * @return The secondary text colour.
     */
    inline wxColour TextSecondary(bool darkMode)
    {
        return darkMode ? wxColour(178, 188, 197) : wxColour(88, 100, 112);
    }

    /**
     * @brief Builds the full path to an asset in the addFiles directory.
     *
     * @param fileName The name of the asset file.
     * @return The resolved path to the requested asset.
     */
    inline wxString GetAssetPath(const wxString &fileName)
    {
        wxString exePath = wxStandardPaths::Get().GetExecutablePath();
        wxFileName exeFile(exePath);
        return exeFile.GetPath() + "/addFiles/" + fileName;
    }

    /**
     * @brief Plays a UI sound effect asynchronously.
     *
     * If the requested sound file exists and loads properly, it is played without blocking the interface.
     *
     * @param fileName The sound file name to play.
     */
    inline void PlayUiSound(const wxString &fileName)
    {
        wxString path = GetAssetPath(fileName);

        if (!wxFileExists(path))
            return;

        wxSound sound(path);
        if (sound.IsOk())
            sound.Play(wxSOUND_ASYNC);
    }

    /**
     * @brief Plays the standard button hover sound.
     */
    inline void PlayButtonHoverSound()
    {
        PlayUiSound("buttonHoverSound.wav");
    }

    /**
     * @brief Plays the standard button click sound.
     */
    inline void PlayButtonClickSound()
    {
        PlayUiSound("buttonClickSound.wav");
    }

    /**
     * @brief Draws a scaled background image and overlay.
     *
     * The image is scaled to fully cover the available area and is then topped with a translucent overlay matching the active theme.
     *
     * @param dc The drawing context used for rendering.
     * @param backgroundBitmap The background image to draw.
     * @param size The size of the destination area.
     * @param darkMode True for dark mode, false for light mode.
     * @param zoom Extra zoom multiplier applied to the background image.
     */
    inline void DrawScaledBackground(
        wxDC &dc,
        const wxBitmap &backgroundBitmap,
        const wxSize &size,
        bool darkMode,
        double zoom = 1.0)
    {
        if (backgroundBitmap.IsOk())
        {
            wxImage bg = backgroundBitmap.ConvertToImage();

            double imgW = static_cast<double>(bg.GetWidth());
            double imgH = static_cast<double>(bg.GetHeight());
            double winW = static_cast<double>(size.GetWidth());
            double winH = static_cast<double>(size.GetHeight());

            double baseScale = std::max(winW / imgW, winH / imgH);
            double finalScale = baseScale * zoom;

            int drawW = static_cast<int>(imgW * finalScale);
            int drawH = static_cast<int>(imgH * finalScale);

            bg.Rescale(drawW, drawH, wxIMAGE_QUALITY_HIGH);

            int x = (size.GetWidth() - drawW) / 2;
            int y = (size.GetHeight() - drawH) / 2;

            dc.DrawBitmap(wxBitmap(bg), x, y, false);
        }
        else
        {
            dc.SetBrush(wxBrush(FrameBackground(darkMode)));
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
        }

        dc.SetBrush(wxBrush(OverlayColour(darkMode)));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
    }

    /**
     * @brief Returns the rectangle for a centered glass panel.
     *
     * @param size The available window size.
     * @param panelHeight The height of the panel to center.
     * @return A rectangle representing the centered panel bounds.
     */
    inline wxRect GetCenteredPanelRect(const wxSize &size, int panelHeight)
    {
        int x = (size.GetWidth() - panelWidth) / 2;
        int y = (size.GetHeight() - panelHeight) / 2;
        return wxRect(x, y, panelWidth, panelHeight);
    }

    /**
     * @brief Draws the main rounded glass panel.
     *
     * @param gc The graphics context used for drawing.
     * @param rect The rectangle describing the panel bounds.
     * @param darkMode True for dark mode, false for light mode.
     */
    inline void DrawGlassPanel(wxGraphicsContext *gc, const wxRect &rect, bool darkMode)
    {
        gc->SetPen(wxPen(PanelBorder(darkMode), 1));
        gc->SetBrush(wxBrush(PanelFill(darkMode)));
        gc->DrawRoundedRectangle(rect.x, rect.y, rect.width, rect.height, panelRadius);
    }

    /**
     * @brief Draws a painted menu button directly onto the menu panel.
     *
     * This helper is used when the launcher menu buttons are rendered directly
     * on the panel instead of being created as child controls.
     *
     * @param gc The graphics context used for drawing.
     * @param rect The rectangle describing the button bounds.
     * @param label The text label drawn inside the button.
     * @param darkMode True for dark mode, false for light mode.
     * @param state The current visual state for the painted button.
     */
    inline void DrawPaintedMenuButton(
        wxGraphicsContext *gc,
        const wxRect &rect,
        const wxString &label,
        bool darkMode,
        const PaintedButtonState &state)
    {
        wxColour fill;
        wxColour hoverFill;
        wxColour pressFill;
        wxColour edge;
        wxColour accent;
        wxColour shadow;
        wxColour textColour = TextPrimary(darkMode);

        if (darkMode)
        {
            wxColour deepBase(8, 14, 28, 248);
            wxColour deepHover(10, 18, 34, 244);
            wxColour deepPress(4, 8, 18, 246);

            wxColour lighterBase(12, 20, 38, 246);
            wxColour lighterHover(14, 24, 44, 248);
            wxColour lighterPress(9, 15, 29, 250);

            fill = state.lighterStyle ? lighterBase : deepBase;
            hoverFill = state.lighterStyle ? lighterHover : deepHover;
            pressFill = state.lighterStyle ? lighterPress : deepPress;

            edge = state.hovered ? wxColour(82, 103, 138, 84) : wxColour(52, 68, 96, 62);
            accent = state.lighterStyle ? wxColour(64, 88, 128) : wxColour(74, 98, 138);
            shadow = wxColour(0, 0, 0, 118);
        }
        else
        {
            wxColour deepBase(232, 238, 247, 246);
            wxColour deepHover(224, 231, 243, 250);
            wxColour deepPress(214, 223, 236, 252);

            wxColour lighterBase(242, 246, 252, 246);
            wxColour lighterHover(232, 238, 248, 250);
            wxColour lighterPress(221, 229, 240, 252);

            fill = state.lighterStyle ? lighterBase : deepBase;
            hoverFill = state.lighterStyle ? lighterHover : deepHover;
            pressFill = state.lighterStyle ? lighterPress : deepPress;

            edge = state.hovered ? wxColour(110, 128, 156, 82) : wxColour(138, 149, 167, 58);
            accent = state.lighterStyle ? wxColour(118, 136, 164) : wxColour(128, 146, 174);
            shadow = wxColour(0, 0, 0, 22);

            textColour = wxColour(34, 44, 60);
        }

        wxColour body = state.pressed ? pressFill : (state.hovered ? hoverFill : fill);

        double x = static_cast<double>(rect.x) + 3.0;
        double y = static_cast<double>(rect.y) + 3.0;
        double drawW = static_cast<double>(rect.width) - 12.0;
        double drawH = static_cast<double>(rect.height) - 13.0;
        const double radius = 17.0;

        if (state.hovered && !state.pressed)
            y -= 1.0;
        if (state.pressed)
            y += 1.0;

        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(wxBrush(shadow));
        gc->DrawRoundedRectangle(x + 6.0, y + 7.0, drawW - 1.0, drawH - 1.0, radius);

        gc->SetBrush(wxBrush(body));
        gc->DrawRoundedRectangle(x, y, drawW, drawH, radius);

        gc->SetPen(wxPen(edge, 1));
        gc->SetBrush(*wxTRANSPARENT_BRUSH);
        gc->DrawRoundedRectangle(x, y, drawW, drawH, radius);

        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(wxBrush(wxColour(
            accent.Red(),
            accent.Green(),
            accent.Blue(),
            10)));
        gc->DrawRoundedRectangle(x + 2.0, y + 2.0, drawW - 4.0, drawH - 4.0, radius - 1.0);

        gc->SetBrush(wxBrush(darkMode ? wxColour(255, 255, 255, 1) : wxColour(255, 255, 255, 10)));
        gc->DrawRoundedRectangle(x + 5.0, y + 5.0, drawW - 10.0, drawH * 0.20, radius - 5.0);

        int buttonWeight = state.hovered ? wxFONTWEIGHT_SEMIBOLD : wxFONTWEIGHT_NORMAL;
        wxFont font(wxFontInfo(13).Family(wxFONTFAMILY_SWISS).Weight(buttonWeight));
        gc->SetFont(font, textColour);

        double tw, th, descent, externalLeading;
        gc->GetTextExtent(label, &tw, &th, &descent, &externalLeading);

        double tx = x + (drawW - tw) / 2.0;
        double ty = y + (drawH - th) / 2.0 - 1.0;

        gc->DrawText(label, tx, ty);
    }

    /**
     * @brief Checks whether a mouse position falls inside a painted menu button.
     *
     * @param rect The rectangle describing the button bounds.
     * @param point The point to test.
     * @return True if the point lies inside the button rectangle.
     */
    inline bool PaintedButtonHitTest(const wxRect &rect, const wxPoint &point)
    {
        return rect.Contains(point);
    }
}

/**
 * @brief Custom painted button control used by the bundl. launcher.
 *
 * This control provides a reusable themed button with hover and press states,
 * optional lighter styling, and button sound effects. It is intended for
 * launcher controls such as the settings back button that should use the same
 * custom rendering approach across different operating systems.
 *
 * @author Aditi Bhardwaj
 */
class MoodyButton : public wxControl
{
public:
    /**
     * @brief Constructs a custom launcher button.
     *
     * Creates the button with the shared launcher size, stores the label and
     * style configuration, and binds the required paint and mouse events.
     *
     * @param parent The parent window that owns this control.
     * @param label The text shown inside the button.
     * @param lighterStyle True to use the lighter style variant, false otherwise.
     */
    MoodyButton(wxWindow *parent, const wxString &label, bool lighterStyle = false)
        : wxControl(),
          m_label(label.Lower()),
          m_lighterStyle(lighterStyle)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);

        Create(
            parent,
            wxID_ANY,
            wxDefaultPosition,
            wxSize(UiHelpers::controlWidth, UiHelpers::controlHeight),
            wxBORDER_NONE);

        Bind(wxEVT_PAINT, &MoodyButton::OnPaint, this);
        Bind(wxEVT_ERASE_BACKGROUND, &MoodyButton::OnEraseBackground, this);
        Bind(wxEVT_ENTER_WINDOW, &MoodyButton::OnEnter, this);
        Bind(wxEVT_LEAVE_WINDOW, &MoodyButton::OnLeave, this);
        Bind(wxEVT_LEFT_DOWN, &MoodyButton::OnLeftDown, this);
        Bind(wxEVT_LEFT_UP, &MoodyButton::OnLeftUp, this);
    }

    /**
     * @brief Updates the button theme mode.
     *
     * Stores the new theme state and refreshes the control so it repaints
     * using the correct colours for light or dark mode.
     *
     * @param enabled True to enable dark mode, false otherwise.
     */
    void SetDarkMode(bool enabled)
    {
        m_darkMode = enabled;
        Refresh();
    }

    /**
     * @brief Prevents the button from receiving keyboard focus.
     *
     * Returning false avoids native focus outlines appearing on top of the
     * custom-painted button on different platforms.
     *
     * @return false always.
     */
    virtual bool AcceptsFocus() const override { return false; }

private:
    /** @brief Lowercase label displayed inside the button. */
    wxString m_label;

    /** @brief Whether the lighter style variant is enabled. */
    bool m_lighterStyle = false;

    /** @brief Whether the button is currently hovered. */
    bool m_hovered = false;

    /** @brief Whether the button is currently pressed. */
    bool m_pressed = false;

    /** @brief Whether dark mode styling is enabled. */
    bool m_darkMode = true;

    /**
     * @brief Prevents the default erase-background behavior.
     *
     * This avoids flicker for the custom-painted button.
     *
     * @param event The erase background event.
     */
    void OnEraseBackground(wxEraseEvent &)
    {
    }

    /**
     * @brief Handles mouse entry into the button.
     *
     * Updates the hover state, plays the hover sound, and refreshes the control.
     *
     * @param event The mouse event.
     */
    void OnEnter(wxMouseEvent &)
    {
        m_hovered = true;
        UiHelpers::PlayButtonHoverSound();
        Refresh();
    }

    /**
     * @brief Handles mouse exit from the button.
     *
     * Clears the hover and pressed states, releases mouse capture if needed,
     * and refreshes the control.
     *
     * @param event The mouse event.
     */
    void OnLeave(wxMouseEvent &)
    {
        m_hovered = false;
        m_pressed = false;

        if (HasCapture())
            ReleaseMouse();

        Refresh();
    }

    /**
     * @brief Handles left mouse button press on the button.
     *
     * Marks the button as pressed, captures the mouse, and refreshes the
     * control so the pressed appearance is drawn.
     *
     * @param event The mouse event.
     */
    void OnLeftDown(wxMouseEvent &)
    {
        m_pressed = true;

        if (!HasCapture())
            CaptureMouse();

        Refresh();
    }

    /**
     * @brief Handles left mouse button release on the button.
     *
     * If the mouse is released inside the button after a press, this plays
     * the click sound and emits a standard button event.
     *
     * @param event The mouse event.
     */
    void OnLeftUp(wxMouseEvent &event)
    {
        bool clickInside = GetClientRect().Contains(event.GetPosition());

        if (HasCapture())
            ReleaseMouse();

        bool shouldClick = m_pressed && clickInside;
        m_pressed = false;
        Refresh();

        if (shouldClick)
        {
            UiHelpers::PlayButtonClickSound();

            wxCommandEvent evt(wxEVT_BUTTON, GetId());
            evt.SetEventObject(this);
            ProcessWindowEvent(evt);
        }
    }

    /**
     * @brief Paints the custom button appearance.
     *
     * Draws the shadow, button body, border, highlight, and centered label
     * using the same styling system as the launcher's other custom UI elements.
     *
     * @param event The paint event.
     */
    void OnPaint(wxPaintEvent &)
    {
        wxAutoBufferedPaintDC dc(this);

        wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
        if (!gc)
            return;

        wxSize sz = GetClientSize();
        const double w = static_cast<double>(sz.GetWidth());
        const double h = static_cast<double>(sz.GetHeight());

        wxColour fill;
        wxColour hoverFill;
        wxColour pressFill;
        wxColour edge;
        wxColour accent;
        wxColour shadow;
        wxColour textColour = UiHelpers::TextPrimary(m_darkMode);

        if (m_darkMode)
        {
            wxColour deepBase(8, 14, 28, 248);
            wxColour deepHover(10, 18, 34, 244);
            wxColour deepPress(4, 8, 18, 246);

            wxColour lighterBase(12, 20, 38, 246);
            wxColour lighterHover(14, 24, 44, 248);
            wxColour lighterPress(9, 15, 29, 250);

            fill = m_lighterStyle ? lighterBase : deepBase;
            hoverFill = m_lighterStyle ? lighterHover : deepHover;
            pressFill = m_lighterStyle ? lighterPress : deepPress;

            edge = m_hovered ? wxColour(82, 103, 138, 84) : wxColour(52, 68, 96, 62);
            accent = m_lighterStyle ? wxColour(64, 88, 128) : wxColour(74, 98, 138);
            shadow = wxColour(0, 0, 0, 118);
        }
        else
        {
            wxColour deepBase(232, 238, 247, 246);
            wxColour deepHover(224, 231, 243, 250);
            wxColour deepPress(214, 223, 236, 252);

            wxColour lighterBase(242, 246, 252, 246);
            wxColour lighterHover(232, 238, 248, 250);
            wxColour lighterPress(221, 229, 240, 252);

            fill = m_lighterStyle ? lighterBase : deepBase;
            hoverFill = m_lighterStyle ? lighterHover : deepHover;
            pressFill = m_lighterStyle ? lighterPress : deepPress;

            edge = m_hovered ? wxColour(110, 128, 156, 82) : wxColour(138, 149, 167, 58);
            accent = m_lighterStyle ? wxColour(118, 136, 164) : wxColour(128, 146, 174);
            shadow = wxColour(0, 0, 0, 22);

            textColour = wxColour(34, 44, 60);
        }

        wxColour body = m_pressed ? pressFill : (m_hovered ? hoverFill : fill);

        double x = 3.0;
        double y = 3.0;
        double drawW = w - 12.0;
        double drawH = h - 13.0;
        const double radius = 17.0;

        if (m_hovered && !m_pressed)
            y -= 1.0;
        if (m_pressed)
            y += 1.0;

        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(wxBrush(shadow));
        gc->DrawRoundedRectangle(x + 6.0, y + 7.0, drawW - 1.0, drawH - 1.0, radius);

        gc->SetBrush(wxBrush(body));
        gc->DrawRoundedRectangle(x, y, drawW, drawH, radius);

        gc->SetPen(wxPen(edge, 1));
        gc->SetBrush(*wxTRANSPARENT_BRUSH);
        gc->DrawRoundedRectangle(x, y, drawW, drawH, radius);

        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(wxBrush(wxColour(
            accent.Red(),
            accent.Green(),
            accent.Blue(),
            10)));
        gc->DrawRoundedRectangle(x + 2.0, y + 2.0, drawW - 4.0, drawH - 4.0, radius - 1.0);

        gc->SetBrush(wxBrush(m_darkMode ? wxColour(255, 255, 255, 1) : wxColour(255, 255, 255, 10)));
        gc->DrawRoundedRectangle(x + 5.0, y + 5.0, drawW - 10.0, drawH * 0.20, radius - 5.0);

        int buttonWeight = m_hovered ? wxFONTWEIGHT_SEMIBOLD : wxFONTWEIGHT_NORMAL;
        wxFont font(wxFontInfo(13).Family(wxFONTFAMILY_SWISS).Weight(buttonWeight));
        gc->SetFont(font, textColour);

        double tw, th, descent, externalLeading;
        gc->GetTextExtent(m_label, &tw, &th, &descent, &externalLeading);

        double tx = x + (drawW - tw) / 2.0;
        double ty = y + (drawH - th) / 2.0 - 1.0;

        gc->DrawText(m_label, tx, ty);

        delete gc;
    }
};

/**
 * @brief Custom toggle row control used in the settings screen.
 *
 * This control displays a label and a themed on/off switch, supports hover feedback, and emits a checkbox-style event when toggled.
 * It is used for the music and dark mode settings in the launcher.
 */
class ToggleRow : public wxControl
{
public:
    /**
     * @brief Constructs a custom toggle row.
     *
     * Creates the labeled toggle control with the provided starting value and binds its paint and mouse events.
     *
     * @param parent The parent window that owns this control.
     * @param label The text label displayed beside the toggle.
     * @param value The initial on or off state.
     */
    ToggleRow(wxWindow *parent, const wxString &label, bool value)
        : wxControl(),
          m_label(label.Lower()),
          m_value(value)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);

        Create(
            parent,
            wxID_ANY,
            wxDefaultPosition,
            wxSize(UiHelpers::controlWidth, UiHelpers::controlHeight),
            wxBORDER_NONE);

        Bind(wxEVT_PAINT, &ToggleRow::OnPaint, this);
        Bind(wxEVT_ERASE_BACKGROUND, &ToggleRow::OnEraseBackground, this);
        Bind(wxEVT_LEFT_UP, &ToggleRow::OnLeftUp, this);
        Bind(wxEVT_ENTER_WINDOW, &ToggleRow::OnEnter, this);
        Bind(wxEVT_LEAVE_WINDOW, &ToggleRow::OnLeave, this);
    }

    /**
     * @brief Updates the toggle row theme mode.
     *
     * @param enabled True to enable dark mode, false otherwise.
     */
    void SetDarkMode(bool enabled)
    {
        m_darkMode = enabled;
        Refresh();
    }

    /**
     * @brief Prevents the toggle row from receiving keyboard focus.
     *
     * Returning false here stops the OS from ever targeting this control with the focus
     * system, which eliminates the native focus rectangle drawn on top of the custom
     * painted appearance on Windows, macOS, and Linux/GTK.
     *
     * @return false always, so the toggle is skipped during tab navigation.
     */
    virtual bool AcceptsFocus() const override { return false; }

private:
    /** @brief Lowercase label displayed for the toggle row. */
    wxString m_label;

    /** @brief Current on or off value of the toggle. */
    bool m_value;

    /** @brief Whether the mouse is currently hovering over the control. */
    bool m_hovered = false;

    /** @brief Whether dark mode styling is enabled. */
    bool m_darkMode = true;

    /**
     * @brief Prevents the default erase-background behavior for the toggle row.
     *
     * @param event The erase background event.
     */
    void OnEraseBackground(wxEraseEvent &)
    {
    }

    /**
     * @brief Handles mouse entry into the toggle row.
     *
     * @param event The mouse event.
     */
    void OnEnter(wxMouseEvent &)
    {
        m_hovered = true;
        Refresh();
    }

    /**
     * @brief Handles mouse exit from the toggle row.
     *
     * @param event The mouse event.
     */
    void OnLeave(wxMouseEvent &)
    {
        m_hovered = false;
        Refresh();
    }

    /**
     * @brief Handles left mouse button release on the toggle row.
     *
     * Flips the stored value, plays the click sound, and emits a checkbox-style command event containing the new state.
     *
     * @param event The mouse event.
     */
    void OnLeftUp(wxMouseEvent &)
    {
        m_value = !m_value;
        UiHelpers::PlayButtonClickSound();
        Refresh();

        wxCommandEvent evt(wxEVT_CHECKBOX, GetId());
        evt.SetInt(m_value ? 1 : 0);
        evt.SetEventObject(this);
        ProcessWindowEvent(evt);
    }

    /**
     * @brief Paints the custom toggle row appearance.
     *
     * Draws the row background, label text, toggle track, and knob based on the current theme and toggle state.
     *
     * @param event The paint event.
     */
    void OnPaint(wxPaintEvent &)
    {
        wxAutoBufferedPaintDC dc(this);

        wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
        if (!gc)
            return;

        wxSize sz = GetClientSize();
        const double w = static_cast<double>(sz.GetWidth());
        const double h = static_cast<double>(sz.GetHeight());
        const double radius = 17.0;

        wxColour fill;
        wxColour hoverFill;
        wxColour edge;
        wxColour accent;
        wxColour textColour = UiHelpers::TextPrimary(m_darkMode);

        wxColour trackOn;
        wxColour trackOff;
        wxColour knob;
        wxColour shadow;

        if (m_darkMode)
        {
            fill = wxColour(8, 14, 28, 248);
            hoverFill = wxColour(10, 18, 34, 244);
            edge = m_hovered ? wxColour(82, 103, 138, 84) : wxColour(52, 68, 96, 62);
            accent = wxColour(74, 98, 138);

            trackOn = wxColour(52, 60, 76);
            trackOff = wxColour(22, 30, 46);
            knob = wxColour(0, 0, 0);
            shadow = wxColour(0, 0, 0, 114);
        }
        else
        {
            fill = wxColour(242, 246, 252, 246);
            hoverFill = wxColour(232, 238, 248, 250);
            edge = m_hovered ? wxColour(110, 128, 156, 82) : wxColour(138, 149, 167, 58);
            accent = wxColour(128, 146, 174);

            trackOn = wxColour(122, 141, 171);
            trackOff = wxColour(198, 206, 219);
            knob = wxColour(255, 255, 255);
            shadow = wxColour(0, 0, 0, 22);

            textColour = wxColour(34, 44, 60);
        }

        wxColour body = m_hovered ? hoverFill : fill;

        double x = 2.0;
        double y = 2.0;
        double drawW = w - 12.0;
        double drawH = h - 13.0;

        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(wxBrush(shadow));
        gc->DrawRoundedRectangle(x + 6.0, y + 7.0, drawW - 1.0, drawH - 1.0, radius);

        gc->SetBrush(wxBrush(body));
        gc->DrawRoundedRectangle(x, y, drawW, drawH, radius);

        gc->SetPen(wxPen(edge, 1));
        gc->SetBrush(*wxTRANSPARENT_BRUSH);
        gc->DrawRoundedRectangle(x, y, drawW, drawH, radius);

        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(wxBrush(wxColour(
            accent.Red(),
            accent.Green(),
            accent.Blue(),
            10)));
        gc->DrawRoundedRectangle(x + 2.0, y + 2.0, drawW - 4.0, drawH - 4.0, radius - 1.0);

        gc->SetBrush(wxBrush(m_darkMode ? wxColour(255, 255, 255, 1) : wxColour(255, 255, 255, 10)));
        gc->DrawRoundedRectangle(x + 5.0, y + 5.0, drawW - 10.0, drawH * 0.22, radius - 5.0);

        int toggleWeight = m_hovered ? wxFONTWEIGHT_SEMIBOLD : wxFONTWEIGHT_NORMAL;
        wxFont labelFont(wxFontInfo(13).Family(wxFONTFAMILY_SWISS).Weight(toggleWeight));
        gc->SetFont(labelFont, textColour);

        double textW, textH, descent, externalLeading;
        gc->GetTextExtent(m_label, &textW, &textH, &descent, &externalLeading);

        double textX = x + 18.0;
        double textY = y + (drawH - textH) / 2.0 - 1.0;
        gc->DrawText(m_label, textX, textY);

        double trackW = 46.0;
        double trackH = 24.0;
        double rightPadding = 20.0;
        double trackX = x + drawW - rightPadding - trackW;
        double trackY = y + (drawH - trackH) / 2.0;

        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(wxBrush(m_value ? trackOn : trackOff));
        gc->DrawRoundedRectangle(trackX, trackY, trackW, trackH, 12.0);

        double knobSize = 20.0;
        double knobX = m_value ? (trackX + trackW - knobSize - 2.0) : (trackX + 2.0);
        double knobY = trackY + 2.0;

        gc->SetBrush(wxBrush(knob));
        gc->DrawEllipse(knobX, knobY, knobSize, knobSize);

        delete gc;
    }
};

#endif