#include <wx/wx.h>
#include "ConnectFourPanel.h"
#include "ConnectFourMenu.h"
#include "ConnectFourAppTheme.h"

/**
 * @class MyFrame
 * @brief Main application window for the Connect Four game.
 * This frame contains the game panel and serves as the primary window displayed to the user after selecting a game mode.
 */
class MyFrame : public wxFrame
{
public:
    /**
     * @brief Constructs the main game window.
     * Initializes the frame, applies the themed size/background, and creates the ConnectFourPanel based on the selected game mode and difficulty.
     * @param mode selected game mode (single player or multiplayer)
     * @param difficulty selected AI difficulty (if applicable)
     */
    MyFrame(GameMode mode, AIDifficulty difficulty)
        : wxFrame(nullptr, wxID_ANY, "Connect Four", wxDefaultPosition, wxSize(920, 780))
    {
        const auto palette = AppTheme::GetPalette();

        SetMinSize(wxSize(860, 720));
        SetBackgroundColour(palette.frameBg);

        new ConnectFourPanel(this, mode, difficulty);
        Centre();
    }
};

/**
 * @class MyApp
 * @brief Main application class for the Connect Four program.
 * Handles application initialization and manages the startup flow, including displaying the menu and launching the game window.
 */
class MyApp : public wxApp
{
public:
    /**
     * @brief Initializes the application.
     * Displays the main menu to the user. If a valid selection is made, creates and shows the main game window.
     * @return true if the application initializes successfully
     * @return false if the user exits from the menu
     */
    virtual bool OnInit()
    {
        ConnectFourMenu menu(nullptr);

        // show the menu first so the user can choose multiplayer or one of the single player difficulties
        if (menu.ShowModal() != wxID_OK)
        {
            return false;
        }

        GameMode selectedMode = menu.getSelectedMode();
        AIDifficulty selectedDifficulty = menu.getSelectedDifficulty();

        MyFrame *frame = new MyFrame(selectedMode, selectedDifficulty);
        frame->Show(true);
        return true;
    }
};

/**
 * @brief Entry point for the wxWidgets application. Initializes and runs the MyApp application.
 */
wxIMPLEMENT_APP(MyApp);