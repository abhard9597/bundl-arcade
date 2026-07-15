#include "MainFrame.h"
#include "MenuPanel.h"
#include "SettingsPanel.h"
#include "UiHelpers.h"

#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/filefn.h>

// Initializes the launcher window, creates the menu and settings panels, configures the main layout, centers the window, loads the background music asset, and prebuilds the supported games.
MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "bundl.", wxDefaultPosition, wxSize(585, 725))
{
    SetBackgroundColour(UiHelpers::FrameBackground(darkMode));
    SetMinSize(wxSize(585, 725));

    menuPanel = new MenuPanel(this);
    settingsPanel = new SettingsPanel(this);

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(menuPanel, 1, wxEXPAND);
    sizer->Add(settingsPanel, 1, wxEXPAND);
    SetSizer(sizer);

    // Start the launcher on the main menu instead of the settings page.
    settingsPanel->Hide();

    Layout();
    Centre(wxBOTH);

    LoadBackgroundMusic();
    PrebuildAllGames();
}

// Temporarily switches the working directory to the requested game folder, runs make synchronously, restores the original working directory, and reports build failures to the user with a message box.
void MainFrame::BuildGame(const wxString &folder, const wxString &displayName)
{
    wxString oldWorkingDir = wxGetCwd();

    if (!wxSetWorkingDirectory(folder))
    {
        wxMessageBox("Failed to access folder for " + displayName + ".", "Build Error", wxOK | wxICON_ERROR);
        return;
    }

    int makeResult = wxExecute("make", wxEXEC_SYNC);

    // Restore the original working directory before returning to the launcher.
    wxSetWorkingDirectory(oldWorkingDir);

    if (makeResult != 0)
    {
        wxMessageBox("Failed to build " + displayName + ".", "Build Error", wxOK | wxICON_ERROR);
    }
}

// This prepares the Snake, Connect Four, and Anagrams executables so they are ready to run when selected from the main menu.
void MainFrame::PrebuildAllGames()
{
    BuildGame("../Snake", "Snake");
    BuildGame("../ConnectFour", "Connect Four");
    BuildGame("../Anagrams", "Anagrams");
}

// Determines the executable directory, looks for the background music asset in the addFiles folder, creates the media controller, and starts playback if the file loads successfully.
void MainFrame::LoadBackgroundMusic()
{
    wxString exePath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName exeFile(exePath);
    wxString exeDir = exeFile.GetPath();

    wxString soundPath = exeDir + "/addFiles/backgroundMusic.wav";

    if (!wxFileExists(soundPath))
        return;

    mediaPlayer = new wxMediaCtrl(this, wxID_ANY);

    if (mediaPlayer->Load(soundPath))
    {
        // Rebind the media finished event so the music can loop continuously.
        mediaPlayer->Bind(wxEVT_MEDIA_FINISHED, &MainFrame::OnMusicFinished, this);
        mediaPlayer->Play();
    }
}

// Resets playback to the beginning of the audio file and starts the music again so that it loops continuously while the launcher is open.
void MainFrame::OnMusicFinished(wxMediaEvent &event)
{
    if (!mediaPlayer)
        return;

    mediaPlayer->Seek(0);
    mediaPlayer->Play();

    event.Skip();
}

// Hides the settings panel, shows the menu panel, refreshes layout, and repaints the menu interface.
void MainFrame::ShowMenu()
{
    settingsPanel->Hide();
    menuPanel->Show();
    Layout();
    menuPanel->Refresh();
}

// Hides the main menu, shows the settings panel, refreshes layout, and repaints the settings interface.
void MainFrame::ShowSettings()
{
    menuPanel->Hide();
    settingsPanel->Show();
    Layout();
    settingsPanel->Refresh();
}

/**
 * Determines the target game directory and executable name based on the provided command string, switches to that directory,
 * exports the current dark mode state through an environment variable, and launches the game asynchronously.
 */
void MainFrame::LaunchGame(const wxString &command)
{
    wxString folder;
    wxString executable;

    if (command.Contains("Snake"))
    {
        folder = "../Snake";
        executable = "./SnakeApp";
    }
    else if (command.Contains("ConnectFour"))
    {
        folder = "../ConnectFour";
        executable = "./ConnectFourApp";
    }
    else if (command.Contains("Anagrams"))
    {
        folder = "../Anagrams";
        executable = "./AnagramsApp";
    }

    if (folder.IsEmpty() || executable.IsEmpty())
        return;

    wxString oldWorkingDir = wxGetCwd();

    if (!wxSetWorkingDirectory(folder))
        return;

    // Pass the current launcher theme to the child game process.
    wxSetEnv("BUNDL_DARK_MODE", darkMode ? "1" : "0");

    wxExecute(executable, wxEXEC_ASYNC);
    wxSetWorkingDirectory(oldWorkingDir);
}

/**
 *Starts background music playback.
 */
void MainFrame::StartMusic()
{
    if (mediaPlayer)
        mediaPlayer->Play();
}

/**
 * Stops background music playback.
 */
void MainFrame::StopMusic()
{
    if (mediaPlayer)
        mediaPlayer->Stop();
}

// Applies the new theme to the frame itself and propagates the theme change to both the menu and settings panels before refreshing the window.
void MainFrame::SetDarkMode(bool enabled)
{
    darkMode = enabled;

    SetBackgroundColour(UiHelpers::FrameBackground(darkMode));

    if (menuPanel)
        menuPanel->SetDarkMode(darkMode);

    if (settingsPanel)
        settingsPanel->SetDarkMode(darkMode);

    Refresh();
    Update();
}

// Returns whether dark mode is currently enabled.
bool MainFrame::IsDarkMode() const
{
    return darkMode;
}