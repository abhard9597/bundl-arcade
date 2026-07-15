/**
 * @file ConnectFourMenu.cpp
 * @brief Implements the themed menu dialog for Connect Four.
 *
 * This file builds the custom card-style menu used to select multiplayer or
 * single-player modes, choose AI difficulty, open the statistics dialog, and
 * close the game.
 *
 * @author Lucas Brown and collaborators
 */

#include "ConnectFourMenu.h"
#include "ConnectFourAppTheme.h"

#include <wx/dcbuffer.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/filefn.h>
#include <wx/stattext.h>
#include <wx/sizer.h>

namespace
{
    const AppTheme::Palette THEME = AppTheme::GetPalette();

    const wxColour BG_FALLBACK = THEME.frameBg;
    const wxColour DARK_OVERLAY = AppTheme::IsDarkMode()
                                      ? wxColour(0, 0, 0, 70)
                                      : wxColour(255, 255, 255, 35);

    const wxColour TILE = THEME.card;
    const wxColour TILE_HOVER = THEME.cardHover;
    const wxColour TILE_PRESS = THEME.cardPress;

    const wxColour TITLE_TEXT = THEME.text;
    const wxColour SUBTITLE_TEXT = THEME.subtleText;
    const wxColour ACCENT = THEME.accent;
    const wxColour SHADOW = THEME.shadow;
    const wxColour LINE = THEME.line;

    const int ID_CARD_1 = wxID_HIGHEST + 1;
    const int ID_CARD_2 = wxID_HIGHEST + 2;
    const int ID_CARD_3 = wxID_HIGHEST + 3;
    const int ID_CARD_4 = wxID_HIGHEST + 4;

    /**
     * @brief Looks for an asset file in a few likely locations.
     * @param fileName name of the file to search for
     * @return wxString path to the file if found, otherwise an empty string
     */
    wxString FindAssetPath(const wxString &fileName)
    {
        wxString exePath = wxStandardPaths::Get().GetExecutablePath();
        wxFileName exeFile(exePath);
        wxString exeDir = exeFile.GetPath();

        wxArrayString candidates;
        candidates.Add(exeDir + "/" + fileName);
        candidates.Add(exeDir + "/addFiles/" + fileName);
        candidates.Add(fileName);
        candidates.Add("addFiles/" + fileName);

        for (const auto &path : candidates)
        {
            if (wxFileExists(path))
            {
                return path;
            }
        }

        return "";
    }

    /**
     * @class MenuCardButton
     * @brief Custom rounded menu card used instead of default wxButtons.
     */
    class MenuCardButton : public wxPanel
    {
    public:
        MenuCardButton(wxWindow *parent, wxWindowID id)
            : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
              m_fill(TILE)
        {
            SetBackgroundStyle(wxBG_STYLE_PAINT);
            SetBackgroundColour(parent->GetBackgroundColour());
            SetCursor(wxCursor(wxCURSOR_HAND));
            SetMinSize(wxSize(540, 84));

            wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

            title = new wxStaticText(this, wxID_ANY, "");
            subtitle = new wxStaticText(this, wxID_ANY, "");

            title->SetForegroundColour(TITLE_TEXT);
            title->SetFont(wxFontInfo(16).Bold().FaceName("Avenir Next"));

            subtitle->SetForegroundColour(SUBTITLE_TEXT);
            subtitle->SetFont(wxFontInfo(11).FaceName("Avenir Next"));

            sizer->AddSpacer(12);
            sizer->Add(title, 0, wxLEFT | wxRIGHT, 18);
            sizer->AddSpacer(4);
            sizer->Add(subtitle, 0, wxLEFT | wxRIGHT, 18);
            SetSizer(sizer);

            Bind(wxEVT_PAINT, &MenuCardButton::OnPaint, this);
            Bind(wxEVT_ENTER_WINDOW, &MenuCardButton::OnEnter, this);
            Bind(wxEVT_LEAVE_WINDOW, &MenuCardButton::OnLeave, this);
            Bind(wxEVT_LEFT_DOWN, &MenuCardButton::OnDown, this);
            Bind(wxEVT_LEFT_UP, &MenuCardButton::OnUp, this);

            title->Bind(wxEVT_LEFT_DOWN, &MenuCardButton::OnDown, this);
            title->Bind(wxEVT_LEFT_UP, &MenuCardButton::OnUp, this);
            subtitle->Bind(wxEVT_LEFT_DOWN, &MenuCardButton::OnDown, this);
            subtitle->Bind(wxEVT_LEFT_UP, &MenuCardButton::OnUp, this);
        }

        /**
         * @brief Updates the text displayed on the card.
         * @param newTitle main card title
         * @param newSubtitle smaller subtitle text
         */
        void SetText(const wxString &newTitle, const wxString &newSubtitle)
        {
            title->SetLabel(newTitle);
            subtitle->SetLabel(newSubtitle);
            Layout();
            Refresh();
        }

    private:
        wxStaticText *title;
        wxStaticText *subtitle;
        wxColour m_fill;
        bool m_pressed = false;

        void OnEnter(wxMouseEvent &event)
        {
            if (!m_pressed)
            {
                m_fill = TILE_HOVER;
                Refresh();
            }
            event.Skip();
        }

        void OnLeave(wxMouseEvent &event)
        {
            if (!m_pressed)
            {
                m_fill = TILE;
                Refresh();
            }
            event.Skip();
        }

        void OnDown(wxMouseEvent &event)
        {
            m_pressed = true;
            m_fill = TILE_PRESS;
            Refresh();

            if (!HasCapture())
            {
                CaptureMouse();
            }

            event.Skip();
        }

        void OnUp(wxMouseEvent &event)
        {
            if (HasCapture())
            {
                ReleaseMouse();
            }

            bool wasPressed = m_pressed;
            m_pressed = false;

            wxPoint p = ScreenToClient(wxGetMousePosition());
            bool inside = GetClientRect().Contains(p);

            m_fill = inside ? TILE_HOVER : TILE;
            Refresh();

            // If the mouse was released inside the card, send a normal button-style event
            if (wasPressed && inside)
            {
                wxCommandEvent clickEvent(wxEVT_BUTTON, GetId());
                clickEvent.SetEventObject(this);
                wxPostEvent(GetParent(), clickEvent);
            }

            event.Skip();
        }

        void OnPaint(wxPaintEvent &)
        {
            wxAutoBufferedPaintDC dc(this);

            wxSize sz = GetClientSize();
            int w = sz.GetWidth();
            int h = sz.GetHeight();
            const int radius = 18;

            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(GetBackgroundColour()));
            dc.DrawRectangle(0, 0, w, h);

            // draw a soft shadow behind the card
            dc.SetBrush(wxBrush(SHADOW));
            dc.DrawRoundedRectangle(6, 7, w - 8, h - 8, radius);

            // draw the main rounded card
            dc.SetBrush(wxBrush(m_fill));
            dc.DrawRoundedRectangle(0, 0, w - 10, h - 10, radius);

            // draw a thin accent strip at the top of the card
            dc.SetBrush(wxBrush(ACCENT));
            dc.DrawRoundedRectangle(0, 0, w - 10, 5, radius);
        }
    };
}

/**
 * @brief Constructs the Connect Four menu dialog.
 * Initializes themed UI elements, loads the background image, binds click events, and displays the main menu screen.
 * @param parent parent window
 */
ConnectFourMenu::ConnectFourMenu(wxWindow *parent)
    : wxDialog(parent, wxID_ANY, "Connect Four", wxDefaultPosition, wxSize(860, 700),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      selectedMode(GameMode::None),
      selectedDifficulty(AIDifficulty::None),
      currentScreen(MenuScreen::Main)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(BG_FALLBACK);
    SetMinSize(wxSize(780, 640));

    // load the menu background image if it exists
    wxString bgPath = FindAssetPath("menu_bg.png");
    if (!bgPath.IsEmpty())
    {
        wxImage img;
        if (img.LoadFile(bgPath))
        {
            m_bgOriginal = wxBitmap(img);
        }
    }

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->AddStretchSpacer(1);

    title = new wxStaticText(this, wxID_ANY, "Connect Four");
    title->SetForegroundColour(TITLE_TEXT);
    title->SetFont(wxFontInfo(30).Bold().FaceName("Avenir Next"));
    title->SetWindowStyleFlag(title->GetWindowStyleFlag() | wxTRANSPARENT_WINDOW);

    subtitle = new wxStaticText(this, wxID_ANY, "A simple arcade game hub style menu");
    subtitle->SetForegroundColour(SUBTITLE_TEXT);
    subtitle->SetFont(wxFontInfo(13).FaceName("Avenir Next"));
    subtitle->SetWindowStyleFlag(subtitle->GetWindowStyleFlag() | wxTRANSPARENT_WINDOW);

    button1 = new MenuCardButton(this, ID_CARD_1);
    button2 = new MenuCardButton(this, ID_CARD_2);
    button3 = new MenuCardButton(this, ID_CARD_3);
    button4 = new MenuCardButton(this, ID_CARD_4);

    mainSizer->Add(title, 0, wxALIGN_CENTER | wxBOTTOM, 8);
    mainSizer->Add(subtitle, 0, wxALIGN_CENTER | wxBOTTOM, 28);
    mainSizer->Add(button1, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxBOTTOM, 14);
    mainSizer->Add(button2, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxBOTTOM, 14);
    mainSizer->Add(button3, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxBOTTOM, 14);
    mainSizer->Add(button4, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT, 14);
    mainSizer->AddStretchSpacer(1);

    SetSizer(mainSizer);
    Centre();

    // Bind normal button-style events instead of raw mouse-up events from the dialog
    Bind(wxEVT_BUTTON, &ConnectFourMenu::OnButton1, this, ID_CARD_1);
    Bind(wxEVT_BUTTON, &ConnectFourMenu::OnButton2, this, ID_CARD_2);
    Bind(wxEVT_BUTTON, &ConnectFourMenu::OnButton3, this, ID_CARD_3);
    Bind(wxEVT_BUTTON, &ConnectFourMenu::OnButton4, this, ID_CARD_4);

    Bind(wxEVT_PAINT, &ConnectFourMenu::OnPaint, this);
    Bind(wxEVT_SIZE, &ConnectFourMenu::OnSize, this);

    ShowMainMenu();
}

/**
 * @brief Updates the title and subtitle text shown on a custom menu card.
 * @param button button panel to update
 * @param titleText main button title
 * @param subtitleText smaller descriptive subtitle
 */
void ConnectFourMenu::SetButtonText(wxPanel *button, const wxString &titleText, const wxString &subtitleText)
{
    MenuCardButton *card = dynamic_cast<MenuCardButton *>(button);
    if (card != nullptr)
    {
        card->SetText(titleText, subtitleText);
    }
}

/**
 * @brief Displays the main menu screen.
 * Sets card labels for selecting game mode or quitting.
 */
void ConnectFourMenu::ShowMainMenu()
{
    currentScreen = MenuScreen::Main;

    title->SetLabel("Connect Four");
    subtitle->SetLabel("Choose a game mode to begin");

    SetButtonText(button1, "Single Player", "Play against the computer");
    SetButtonText(button2, "Multiplayer", "Two players on the same board");
    SetButtonText(button3, "Statistics", "View saved results on this device");
    SetButtonText(button4, "Quit", "Close the game");
}

/**
 * @brief Displays the difficulty selection screen.
 * Allows the user to choose AI difficulty or return to the main menu.
 */
void ConnectFourMenu::ShowDifficultyMenu()
{
    currentScreen = MenuScreen::Difficulty;

    title->SetLabel("Choose Difficulty");
    subtitle->SetLabel("Select how the computer should play");

    SetButtonText(button1, "Normal", "The computer makes random moves");
    SetButtonText(button2, "Hard", "The computer makes smarter moves sometimes");
    SetButtonText(button3, "Back", "Return to the previous menu");
    SetButtonText(button4, "Quit", "Close the game");
}

/**
 * @brief Handles the first button click event.
 * Behavior depends on the current menu screen:
 * - main menu: navigates to difficulty selection
 * - difficulty menu: selects normal difficulty and starts the game
 * @param event button click event
 */
void ConnectFourMenu::OnButton1(wxCommandEvent &event)
{

    if (currentScreen == MenuScreen::Main)
    {
        soundManager.PlayMenuButtonSound();
        ShowDifficultyMenu();
    }
    else if (currentScreen == MenuScreen::Difficulty)
    {
        selectedMode = GameMode::SinglePlayer;
        selectedDifficulty = AIDifficulty::Normal;

        // Delay closing the dialog until the current event finishes
        EndModal(wxID_OK);
    }
}

/**
 * @brief Handles the second button click event.
 * Behavior depends on the current menu screen:
 * - main menu: selects multiplayer mode
 * - difficulty menu: selects hard difficulty and starts the game
 * @param event button click event
 */
void ConnectFourMenu::OnButton2(wxCommandEvent &event)
{
    soundManager.PlayMenuButtonSound();

    if (currentScreen == MenuScreen::Main)
    {
        selectedMode = GameMode::MultiPlayer;
        selectedDifficulty = AIDifficulty::None;

        // Delay closing the dialog until the current event finishes
        EndModal(wxID_OK);
    }
    else if (currentScreen == MenuScreen::Difficulty)
    {
        selectedMode = GameMode::SinglePlayer;
        selectedDifficulty = AIDifficulty::Hard;

        // Delay closing the dialog until the current event finishes
        EndModal(wxID_OK);
    }
}

/**
 * @brief Handles the third button click event.
 * Behavior depends on the current menu screen:
 * - main menu: opens the statistics dialog
 * - difficulty menu: returns to the main menu
 * @param event button click event
 */
void ConnectFourMenu::OnButton3(wxCommandEvent &event)
{
    soundManager.PlayMenuButtonSound();

    if (currentScreen == MenuScreen::Main)
    {
        ConnectFourStatsManager statsManager;
        statsManager.Load(ConnectFourStatsManager::GetDefaultFilePath());

        ConnectFourStatsDialog statsDialog(
            this,
            statsManager.Get(),
            []()
            {
                ConnectFourStatsManager refreshedStats;
                refreshedStats.Load(ConnectFourStatsManager::GetDefaultFilePath());
                return refreshedStats.Get();
            },
            []()
            {
                ConnectFourStatsManager refreshedStats;
                refreshedStats.Load(ConnectFourStatsManager::GetDefaultFilePath());
                refreshedStats.Reset();
            });

        statsDialog.ShowModal();
    }
    else if (currentScreen == MenuScreen::Difficulty)
    {
        ShowMainMenu();
    }
}

/**
 * @brief Handles the fourth button click event.
 *
 * This button always closes the menu dialog without starting a game. The
 * selected mode and difficulty are reset so the caller can detect that the user
 * chose to quit.
 *
 * @param event button click event
 */
void ConnectFourMenu::OnButton4(wxCommandEvent &event)
{
    soundManager.PlayMenuButtonSound();

    selectedMode = GameMode::None;
    selectedDifficulty = AIDifficulty::None;
    EndModal(wxID_CANCEL);
}

/**
 * @brief Handles painting the menu background.
 * Draws the scaled background image if available, otherwise uses a solid fallback color.
 * @param event paint event
 */
void ConnectFourMenu::OnPaint(wxPaintEvent &event)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

    wxSize sz = GetClientSize();

    if (m_bgOriginal.IsOk())
    {
        if (!m_bgScaled.IsOk() ||
            m_bgScaled.GetWidth() != sz.GetWidth() ||
            m_bgScaled.GetHeight() != sz.GetHeight())
        {
            wxImage scaled = m_bgOriginal.ConvertToImage();
            scaled.Rescale(sz.GetWidth(), sz.GetHeight(), wxIMAGE_QUALITY_HIGH);
            m_bgScaled = wxBitmap(scaled);
        }

        dc.DrawBitmap(m_bgScaled, 0, 0, false);

        // draw a dark overlay so the text is easier to read
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(DARK_OVERLAY));
        dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());
    }
    else
    {
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(BG_FALLBACK));
        dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());
    }
}

/**
 * @brief Handles resizing the menu window.
 * Clears the cached scaled image so the background can be rebuilt at the new size.
 * @param event size event
 */
void ConnectFourMenu::OnSize(wxSizeEvent &event)
{
    m_bgScaled = wxBitmap();
    Refresh();
    event.Skip();
}

/**
 * @brief Gets the selected game mode.
 * @return GameMode selected mode
 */
GameMode ConnectFourMenu::getSelectedMode() const
{
    return selectedMode;
}

/**
 * @brief Gets the selected AI difficulty.
 * @return AIDifficulty selected difficulty
 */
AIDifficulty ConnectFourMenu::getSelectedDifficulty() const
{
    return selectedDifficulty;
}