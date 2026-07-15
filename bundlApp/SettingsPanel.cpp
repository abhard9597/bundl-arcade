#include "SettingsPanel.h"
#include "MainFrame.h"
#include "UiHelpers.h"
#include <wx/dcbuffer.h>

// Initializes the settings interface, loads the shared background image, creates the title, subtitle, toggle controls, and back button, binds their events, and applies the current theme styling.
SettingsPanel::SettingsPanel(MainFrame *parent)
    : wxPanel(parent), frame(parent), darkMode(parent->IsDarkMode())
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetDoubleBuffered(true);

    wxString bgPath = UiHelpers::GetAssetPath("menuBackground.png");
    if (wxFileExists(bgPath))
        backgroundBitmap.LoadFile(bgPath, wxBITMAP_TYPE_PNG);

    wxBoxSizer *rootSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *blockSizer = new wxBoxSizer(wxVERTICAL);

    titleText = new wxStaticText(this, wxID_ANY, "settings");
    wxFont titleFont(23, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false);
    titleText->SetFont(titleFont);

    subtitleText = new wxStaticText(this, wxID_ANY, "your preferences");
    wxFont subtitleFont(13, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT, false);
    subtitleText->SetFont(subtitleFont);

    musicToggle = new ToggleRow(this, "background music", true);
    darkModeToggle = new ToggleRow(this, "dark mode", darkMode);
    backButton = new MoodyButton(this, "back", true);

    blockSizer->Add(titleText, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 6);
    blockSizer->Add(subtitleText, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 20);
    blockSizer->Add(musicToggle, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, UiHelpers::controlGap);
    blockSizer->Add(darkModeToggle, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, UiHelpers::controlGap);
    blockSizer->Add(backButton, 0, wxALIGN_CENTER_HORIZONTAL);

    // Center the settings content vertically within the glass panel.
    rootSizer->AddStretchSpacer(1);
    rootSizer->Add(blockSizer, 0, wxALIGN_CENTER_HORIZONTAL);
    rootSizer->AddStretchSpacer(1);

    SetSizer(rootSizer);

    musicToggle->Bind(wxEVT_CHECKBOX, &SettingsPanel::OnToggleMusic, this);
    darkModeToggle->Bind(wxEVT_CHECKBOX, &SettingsPanel::OnToggleDarkMode, this);
    backButton->Bind(wxEVT_BUTTON, &SettingsPanel::OnBack, this);

    Bind(wxEVT_PAINT, &SettingsPanel::OnPaint, this);

    ApplyTheme();
}

// Reapplies theme-specific colours and repaints the panel.
void SettingsPanel::SetDarkMode(bool enabled)
{
    darkMode = enabled;
    ApplyTheme();
    Refresh();
}

// Updates text colours and forwards the current theme state to the custom toggle controls and back button.
void SettingsPanel::ApplyTheme()
{
    if (titleText)
        titleText->SetForegroundColour(UiHelpers::TextPrimary(darkMode));

    if (subtitleText)
        subtitleText->SetForegroundColour(UiHelpers::TextSecondary(darkMode));

    if (musicToggle)
        musicToggle->SetDarkMode(darkMode);

    if (darkModeToggle)
        darkModeToggle->SetDarkMode(darkMode);

    if (backButton)
        backButton->SetDarkMode(darkMode);
}

// Draws the scaled background image and then renders the centered glass-style content panel used behind the settings controls.
void SettingsPanel::OnPaint(wxPaintEvent &)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

    wxSize sz = GetClientSize();
    UiHelpers::DrawScaledBackground(dc, backgroundBitmap, sz, darkMode, 1.0);

    wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
    if (!gc)
        return;

    wxRect panelRect = UiHelpers::GetCenteredPanelRect(sz, UiHelpers::settingsPanelHeight);
    UiHelpers::DrawGlassPanel(gc, panelRect, darkMode);

    delete gc;
}

// Starts music when the toggle is checked and stops music when it is unchecked.
void SettingsPanel::OnToggleMusic(wxCommandEvent &event)
{
    if (event.IsChecked())
        frame->StartMusic();
    else
        frame->StopMusic();
}

// Updates the launcher theme and synchronizes the new theme state with the parent frame.
void SettingsPanel::OnToggleDarkMode(wxCommandEvent &event)
{
    darkMode = event.IsChecked();
    frame->SetDarkMode(darkMode);
}

// Requests that the parent frame return to the main menu.
void SettingsPanel::OnBack(wxCommandEvent &)
{
    frame->ShowMenu();
}