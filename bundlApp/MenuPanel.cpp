#include "MenuPanel.h"
#include "MainFrame.h"
#include "UiHelpers.h"
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>

// Initializes the menu interface, loads the shared background image, prepares the painted button states, binds mouse and paint events, and applies the current theme styling.
MenuPanel::MenuPanel(MainFrame *parent)
    : wxPanel(parent), frame(parent), darkMode(parent->IsDarkMode())
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetDoubleBuffered(true);

    wxString bgPath = UiHelpers::GetAssetPath("menuBackground.png");
    if (wxFileExists(bgPath))
        backgroundBitmap.LoadFile(bgPath, wxBITMAP_TYPE_PNG);

    buttonStates[exitIndex].lighterStyle = true;

    LayoutButtonRects();

    Bind(wxEVT_PAINT, &MenuPanel::OnPaint, this);
    Bind(wxEVT_SIZE, &MenuPanel::OnSize, this);
    Bind(wxEVT_MOTION, &MenuPanel::OnMouseMove, this);
    Bind(wxEVT_LEFT_DOWN, &MenuPanel::OnLeftDown, this);
    Bind(wxEVT_LEFT_UP, &MenuPanel::OnLeftUp, this);
    Bind(wxEVT_LEAVE_WINDOW, &MenuPanel::OnMouseLeave, this);

    ApplyTheme();
}

// Reapplies theme-specific colours and repaints the panel.
void MenuPanel::SetDarkMode(bool enabled)
{
    darkMode = enabled;
    ApplyTheme();
    Refresh();
}

// Updates the painted menu so it redraws using the currently active theme.
void MenuPanel::ApplyTheme()
{
    Refresh();
}

// Updates the button layout so the painted buttons stay centered inside the glass panel.
void MenuPanel::LayoutButtonRects()
{
    wxSize sz = GetClientSize();
    wxRect panelRect = UiHelpers::GetCenteredPanelRect(sz, UiHelpers::menuPanelHeight);

    int buttonX = panelRect.x + (panelRect.width - UiHelpers::controlWidth) / 2;
    int firstButtonY = panelRect.y + 108;

    for (int i = 0; i < buttonCount; ++i)
    {
        int buttonY = firstButtonY + i * (UiHelpers::controlHeight + UiHelpers::controlGap);
        buttonRects[i] = wxRect(buttonX, buttonY, UiHelpers::controlWidth, UiHelpers::controlHeight);
    }
}

// Returns the index of the painted menu button under the mouse, or -1 if no button is hit.
int MenuPanel::HitTestButton(const wxPoint &point) const
{
    for (int i = 0; i < buttonCount; ++i)
    {
        if (UiHelpers::PaintedButtonHitTest(buttonRects[i], point))
            return i;
    }

    return -1;
}

// Clears the pressed appearance from every painted menu button.
void MenuPanel::ClearPressedState()
{
    for (auto &state : buttonStates)
        state.pressed = false;
}

// Clears the hovered appearance from every painted menu button.
void MenuPanel::ClearHoveredState()
{
    for (auto &state : buttonStates)
        state.hovered = false;
}

// Activates the action associated with the selected painted button.
void MenuPanel::ActivateButton(int buttonIndex)
{
    switch (buttonIndex)
    {
    case snakeIndex:
        OnSnake();
        break;
    case connectIndex:
        OnConnectFour();
        break;
    case anagramsIndex:
        OnAnagrams();
        break;
    case settingsIndex:
        OnSettings();
        break;
    case exitIndex:
        OnExit();
        break;
    default:
        break;
    }
}

// Draws the scaled background image and then renders the centered glass-style content panel, title text, subtitle text, and painted menu buttons.
void MenuPanel::OnPaint(wxPaintEvent &)
{
    wxAutoBufferedPaintDC baseDc(this);
    wxGCDC dc(baseDc);

    dc.Clear();

    LayoutButtonRects();

    wxSize sz = GetClientSize();
    UiHelpers::DrawScaledBackground(dc, backgroundBitmap, sz, darkMode, 1.0);

    wxGraphicsContext *gc = dc.GetGraphicsContext();
    if (!gc)
        return;

    wxRect panelRect = UiHelpers::GetCenteredPanelRect(sz, UiHelpers::menuPanelHeight);
    UiHelpers::DrawGlassPanel(gc, panelRect, darkMode);

    dc.SetTextForeground(UiHelpers::TextPrimary(darkMode));
    dc.SetFont(wxFont(28, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    wxString title = "bundl.";
    wxSize titleSize = dc.GetTextExtent(title);
    int titleX = panelRect.x + (panelRect.width - titleSize.GetWidth()) / 2;
    int titleY = panelRect.y + 40;
    dc.DrawText(title, titleX, titleY);

    dc.SetTextForeground(UiHelpers::TextSecondary(darkMode));
    dc.SetFont(wxFont(13, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    wxString subtitle = "your move";
    wxSize subtitleSize = dc.GetTextExtent(subtitle);
    int subtitleX = panelRect.x + (panelRect.width - subtitleSize.GetWidth()) / 2;
    int subtitleY = titleY + titleSize.GetHeight() + 2;
    dc.DrawText(subtitle, subtitleX, subtitleY);

    static const wxString buttonLabels[buttonCount] = {
        "snake",
        "connect four",
        "anagrams",
        "settings",
        "exit"};

    for (int i = 0; i < buttonCount; ++i)
    {
        UiHelpers::DrawPaintedMenuButton(
            gc,
            buttonRects[i],
            buttonLabels[i],
            darkMode,
            buttonStates[i]);
    }
}

// Recomputes button layout whenever the menu panel is resized.
void MenuPanel::OnSize(wxSizeEvent &event)
{
    LayoutButtonRects();
    Refresh();
    event.Skip();
}

// Updates hover and pressed visual states while the mouse moves across the painted menu buttons.
void MenuPanel::OnMouseMove(wxMouseEvent &event)
{
    int newHovered = HitTestButton(event.GetPosition());
    bool changed = (newHovered != hoveredButton);

    if (changed && newHovered != -1)
        UiHelpers::PlayButtonHoverSound();

    hoveredButton = newHovered;

    ClearHoveredState();
    if (hoveredButton != -1)
        buttonStates[hoveredButton].hovered = true;

    ClearPressedState();
    if (pressedButton != -1 && hoveredButton == pressedButton && event.LeftIsDown())
        buttonStates[pressedButton].pressed = true;

    if (changed || pressedButton != -1)
        Refresh();
}

// Marks a painted button as pressed when the mouse button is pressed inside it.
void MenuPanel::OnLeftDown(wxMouseEvent &event)
{
    int hitButton = HitTestButton(event.GetPosition());

    ClearPressedState();
    pressedButton = hitButton;

    if (pressedButton != -1)
    {
        buttonStates[pressedButton].pressed = true;

        if (!HasCapture())
            CaptureMouse();

        Refresh();
    }
}

// Activates a painted button if the mouse is released inside the same button that was pressed.
void MenuPanel::OnLeftUp(wxMouseEvent &event)
{
    int releasedButton = HitTestButton(event.GetPosition());
    bool shouldActivate = (pressedButton != -1 && pressedButton == releasedButton);

    if (HasCapture())
        ReleaseMouse();

    ClearPressedState();
    pressedButton = -1;

    if (shouldActivate)
    {
        UiHelpers::PlayButtonClickSound();
        ActivateButton(releasedButton);
    }

    Refresh();
}

// Clears hover feedback when the mouse pointer leaves the menu panel.
void MenuPanel::OnMouseLeave(wxMouseEvent &)
{
    hoveredButton = -1;
    ClearHoveredState();

    if (!HasCapture())
    {
        ClearPressedState();
        pressedButton = -1;
    }
    else
    {
        ClearPressedState();
    }

    Refresh();
}

// Requests that the parent frame launch the Snake game.
void MenuPanel::OnSnake()
{
    frame->LaunchGame("Snake");
}

// Requests that the parent frame launch the Connect Four game.
void MenuPanel::OnConnectFour()
{
    frame->LaunchGame("ConnectFour");
}

// Requests that the parent frame launch the Anagrams game.
void MenuPanel::OnAnagrams()
{
    frame->LaunchGame("Anagrams");
}

// Requests that the parent frame switch to the settings panel.
void MenuPanel::OnSettings()
{
    frame->ShowSettings();
}

// Requests that the main application frame close.
void MenuPanel::OnExit()
{
    frame->Close(true);
}
